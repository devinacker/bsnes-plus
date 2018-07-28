#ifdef BSX_CPP

BSXBase bsxbase;

void BSXBase::Enter() { bsxbase.enter(); }

void BSXBase::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    // buffer a packet for currently open streams
    for(auto &stream : regs.stream) {
      if(stream.queue) {
        stream.queue--;
        if(stream.pf_latch && stream.pf_queue < 0x80) stream.pf_queue++;
        if(stream.dt_latch && stream.dt_queue < 0x80) stream.dt_queue++;
      }
    }

    // time step based on BT.1126 layer 2
    // simulate an estimated number of bits to buffer full link-layer packets:
    // 30 bit header + 176 bit payload + 82 bit error correction/CRC per packet
    // right now, act as if there are only ever two total channels in the broadcast,
    // and that they're both using equal bandwidth, but in reality, there could be any
    // arbitrary number of channels being broadcast asynchronously with each other in 
    // the same satellite data stream
    step(288*2);
    synchronize_cpu();
  }
}

void BSXBase::init() {
}

void BSXBase::enable() {
  memory::mmio.map(0x2188, 0x219f, *this);
}

void BSXBase::power() {
  reset();
}

void BSXBase::reset() {
  // data clock based on BT.1126 layer 1 (for NTSC mode B)
  // 224 data bits and 48 16-bit stereo samples per (physical) frame
  // 1000 frames per sec = 224kbit/s data and 48kHz audio
  create(BSXBase::Enter, 224*1000);

  memset(&regs, 0x00, sizeof regs);
  
  local_time = config.sat.local_time;
  custom_time = config.sat.custom_time;

  regs.r2196 = 0x10;
  regs.r2197 = 0x80;

  time(&start_time);
}

void BSXBase::unload() {
  regs.stream[0].packets.close();
  regs.stream[1].packets.close();
}

bool BSXBase::stream_fileload(BSXStream &stream)
{
  //Make sure to close the file first
  stream.packets.close();
  char filename[256];
  string filepath;
  sprintf(filename, "BSX%04X-%d.bin", stream.channel, stream.count);
  filepath << config.sat.path << filename;
  
  //Open Satellaview file
  if (stream.packets.open(filepath, file::mode::read))
  {
    stream.first = true;
    stream.loaded_channel = stream.channel;
    stream.loaded_count = stream.count;
    stream.queue = ceil(stream.packets.size() / 22.);
  }
  else
  {
    stream.loaded_channel = 0;
  }
  
  return stream.packets.open();
}

uint8 BSXBase::get_time(BSXStream &stream)
{
  if (stream.offset == 0) {
    time_t rawtime;
    tm *t;
    
    time(&rawtime);
    if (local_time) {
      t = localtime(&rawtime);
    } else {
      rawtime -= start_time;
      rawtime += custom_time;
      t = gmtime(&rawtime);
    }

    // adjust time to BS-X value ranges
    t->tm_wday++;
    t->tm_mon++;
    t->tm_year += 1900;
    // store time for current stream
    stream.time = *t;
  }

  switch(stream.offset) {
    case  0: return 0x00;  //Data Group ID / Repetition
    case  1: return 0x00;  //Data Group Link / Continuity
    case  2: return 0x00;  //Data Group Size (24-bit)
    case  3: return 0x00;
    case  4: return 0x10;
    case  5: return 0x01;  //Must be 0x01
    case  6: return 0x01;  //Amount of packets (1)
    case  7: return 0x00;  //Offset (24-bit)
    case  8: return 0x00;
    case  9: return 0x00;
    case 10: return stream.time.tm_sec;
    case 11: return stream.time.tm_min;
    case 12: return stream.time.tm_hour;
    case 13: return stream.time.tm_wday;
    case 14: return stream.time.tm_mday;
    case 15: return stream.time.tm_mon;
    case 16: return stream.time.tm_year >> 0;
    case 17: return stream.time.tm_year >> 8;
    default: return 0x00;
  }
}

uint8 BSXBase::mmio_read(unsigned addr) {
  if(!Memory::debugger_access())
    cpu.synchronize_coprocessor();

  unsigned streamnum = addr >= 0x218e ? 1 : 0;
  BSXStream &stream = regs.stream[streamnum];

  switch(addr) {
    //Stream 1 & 2
    case 0x2188: 
    case 0x218e: 
      return stream.channel >> 0; //Logical Channel 1 + Data Structure
    case 0x2189: 
    case 0x218f: 
      return stream.channel >> 8; //Logical Channel 2

    case 0x218a:
    case 0x2190: {
      //Prefix Count
      if (!stream.pf_latch || !stream.dt_latch)
      {
        //Stream Not Enabled
        return 0;
      }

      if (!stream.pf_queue && !stream.dt_queue && !Memory::debugger_access())
      {
        //Queue is empty
        stream.offset = 0;
        if (stream.channel == 0)
        {
          //Time Channel, one packet
          stream.first = true;
          stream.loaded_channel = 0;
          stream.queue = 1;
        }
        else if (!stream_fileload(stream) && stream.count > 0) 
        {
          stream.count = 0;
          stream_fileload(stream);
        }
        stream.count++;
      }
      
      return stream.pf_queue;
    }
    
    case 0x218b:
    case 0x2191: {
      //Prefix Data Latch
      if (stream.pf_latch)
      {
        //Latch enabled
        if(stream.pf_queue && !Memory::debugger_access())
        {
          stream.prefix = 0;
          if (stream.first)
          {
            //First packet
            stream.prefix |= 0x10;
            stream.first = false;
          }

          stream.pf_queue--;
          if (stream.queue == 0 && stream.pf_queue == 0)
          {
            //Last packet
            stream.prefix |= 0x80;
          }

          stream.status |= stream.prefix;
        }

        return stream.prefix;
      }
      else
      {
        //Latch not enabled
        return 0;
      }
    }
    
    case 0x218c:
    case 0x2192: {
      //Data Latch
      if (stream.dt_latch)
      {
        if(stream.dt_queue && !Memory::debugger_access())
        {
          if (stream.channel == 0)
          {
            //Return Time
            stream.data = get_time(stream);
          }
          else if (stream.packets.open())
          {
            //Get packet data
            stream.data = stream.packets.read();
          }

          stream.offset++;
          if(stream.offset % 22 == 0)
          {
            //finished reading current packet
            stream.dt_queue--;
          }
        }
        
        return stream.data;
      }
      else
      {
        return 0;
      }
    }
    
    case 0x218d:
    case 0x2193: {
      //Prefix Data OR Gate
      uint8 temp = stream.status;
      if((regs.r2194 & 1) && !Memory::debugger_access())
      {
        stream.status = 0;
      }
      return temp; 
    }
    
    //Other
    case 0x2194: return regs.r2194; //Satellaview LED and Stream register
    case 0x2195: return regs.r2195; //Unknown
    case 0x2196: return regs.r2196; //Satellaview Status
    case 0x2197: return regs.r2197; //Soundlink
    case 0x2198: return regs.r2198; //Serial I/O - Serial Number
    case 0x2199: return regs.r2199; //Serial I/O - ???
  }

  return cpu.regs.mdr;
}

void BSXBase::mmio_write(unsigned addr, uint8 data) {
  if(!Memory::debugger_access())
    cpu.synchronize_coprocessor();

  unsigned streamnum = addr >= 0x218e ? 1 : 0;
  BSXStream &stream = regs.stream[streamnum];

  switch(addr) {
    //Stream 1 & 2
    case 0x2188:
    case 0x218e: {
      //Logical Channel 1 + Data Structure
      if ((stream.channel & 0xff) != data)
        stream.count = 0;
      stream.channel = (stream.channel & 0x3f00) | data;
    } break;

    case 0x2189:
    case 0x218f: {
      //Logical Channel 2 (6bit)
      if ((stream.channel >> 8) != (data & 0x3F))
        stream.count = 0;
      stream.channel = (stream.channel & 0xff) | (data << 8);
    } break;

    case 0x218b:
    case 0x2191: {
      //Prefix Data Latch
      stream.pf_latch = (data != 0);
      stream.pf_queue = 0;
    } break;

    case 0x218c:
    case 0x2192: {
      //Data Latch
      stream.dt_latch = (data != 0);
      stream.dt_queue = 0;
    } break;


    //Other
    case 0x2194: {
      //Satellaview LED and Stream enable (4bit)
      regs.r2194 = data & 0x0F;
    } break;

    case 0x2197: {
      //Soundlink
      regs.r2197 = data;
    } break;
  }
}

#endif

