#ifdef BSX_CPP

BSXBase bsxbase;

void BSXBase::init() {
}

void BSXBase::enable() {
  memory::mmio.map(0x2188, 0x219f, *this);
}

void BSXBase::power() {
  reset();
}

void BSXBase::reset() {
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

void BSXBase::stream_fileload(BSXStream &stream)
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
    stream.count++;
    stream.queue = ceil(stream.packets.size() / 22.);
  }
  else if (stream.count > 0)
  {
    stream.count = 0;
    stream_fileload(stream);
  }
}

uint8 BSXBase::get_time()
{
  unsigned counter = regs.time_counter;
  regs.time_counter++;
  if (regs.time_counter >= 22)
    regs.time_counter = 0;

  if (counter == 0) {
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

    regs.time_hour   = t->tm_hour;
    regs.time_minute = t->tm_min;
    regs.time_second = t->tm_sec;
    regs.time_weekday = (t->tm_wday) + 1;
    regs.time_day = t->tm_mday;
    regs.time_month = (t->tm_mon) + 1;
    regs.time_year = (t->tm_year) + 1900;
  }

  switch(counter) {
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
    case 10: return regs.time_second;
    case 11: return regs.time_minute;
    case 12: return regs.time_hour;
    case 13: return regs.time_weekday;
    case 14: return regs.time_day;
    case 15: return regs.time_month;
    case 16: return regs.time_year >> 0;
    case 17: return regs.time_year >> 8;
    default: return 0x00;
  }
}

uint8 BSXBase::mmio_read(unsigned addr) {
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

      if (stream.channel == 0)
      {
        //Time Channel, one packet
        return 0x01;
      }
      else if (stream.queue == 0 && !Memory::debugger_access())
      {
        //Queue is empty
        stream_fileload(stream);
      }
      
      //Lock max value at 0x7F for bigger packets
      return min(stream.queue, 0x7f);
    }
    
    case 0x218b:
    case 0x2191: {
      //Prefix Data Latch
      if (stream.pf_latch)
      {
        //Latch enabled
        if (stream.channel == 0)
        {
          //Time Channel, only one packet, both start and end
          stream.prefix = 0x90;
        }
        else if(stream.packets.open() && !Memory::debugger_access())
        {
          stream.prefix = 0;
          if (stream.first)
          {
            //First packet
            stream.prefix |= 0x10;
            stream.first = false;
          }

          if (stream.queue > 0)
          {
            stream.queue--;
          }
          if (stream.queue == 0)
          {
            //Last packet
            stream.prefix |= 0x80;
          }
        }

        stream.prefix_or |= stream.prefix;
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
        if(!Memory::debugger_access())
        {
          if (stream.channel == 0)
          {
            //Return Time
            stream.data = get_time();
          }
          else if (stream.packets.open())
          {
            //Get packet data
            stream.data = stream.packets.read();
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
      uint8 temp = stream.prefix_or;
      if((regs.r2194 & 1) && !Memory::debugger_access())
      {
        stream.prefix_or = 0;
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
    } break;

    case 0x218c:
    case 0x2192: {
      //Data Latch
      if (stream.channel == 0 && !Memory::debugger_access())
      {
        //Hardcoded Time Channel
        regs.time_counter = 0;
      }
      stream.dt_latch = (data != 0);
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

