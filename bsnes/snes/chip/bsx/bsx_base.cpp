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

void BSXBase::stream1_fileload(uint8 count)
{
  //Make sure to close the file first
  SAT1.close();
  char filename[256];
  string filepath;
  sprintf(filename, "BSX%04X-%d.bin", (regs.r2188 | (regs.r2189 * 256)), count);
  filepath << config.sat.path << filename;
  
  //Open Satellaview file
  if (SAT1.open(filepath, file::mode::read))
  {
    regs.stream1_loaded = true;
    regs.stream1_first = true;
    float QueueSize = SAT1.size() / 22.;
    regs.stream1_queue = ceil(QueueSize);

    regs.stream1_first = true;
  }
  else
  {
    regs.stream1_loaded = false;
  }
}

void BSXBase::stream2_fileload(uint8 count)
{
  //Make sure to close the file first
  SAT2.close();
  char filename[256];
  string filepath;
  sprintf(filename, "BSX%04X-%d.bin", (regs.r218e | (regs.r218f * 256)), count);
  filepath << config.sat.path << filename;

  //Open Satellaview file
  if (SAT2.open(filepath, file::mode::read))
  {
    regs.stream2_loaded = true;
    regs.stream2_first = true;
    float QueueSize = SAT1.size() / 22.;
    regs.stream2_queue = ceil(QueueSize);

    regs.stream2_first = true;
  }
  else
  {
    regs.stream2_loaded = false;
  }
}

uint8 BSXBase::get_time(bool reset)
{
  if(reset == true) {
    regs.time_counter = 0;
    return 0xff;
  }
  
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
    uint16 time_year = (t->tm_year) + 1900;
    regs.time_yearL = time_year & 0xFF;
    regs.time_yearH = time_year >> 8;
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
    case 16: return regs.time_yearL;
    case 17: return regs.time_yearH;
    default: return 0x00;
  }
}

uint8 BSXBase::mmio_read(unsigned addr) {
  switch(addr) {
    //Stream 1
    case 0x2188: return regs.r2188; //Logical Channel 1 + Data Structure
    case 0x2189: return regs.r2189; //Logical Channel 2

    case 0x218a: {
      //Prefix Count
      if (!regs.pf_latch1_enable || !regs.dt_latch1_enable)
      {
        //Stream Not Enabled
        return 0;
      }

      if (regs.r2188 == 0 && regs.r2189 == 0)
      {
        //Time Channel, one packet
        return 0x01;
      }
      
      if(!Memory::debugger_access())
      {
        if (regs.stream1_queue <= 0)
        {
          //Queue is empty
          regs.stream1_count++;
          stream1_fileload(regs.stream1_count - 1);
        }
      
        if (!regs.stream1_loaded && (regs.stream1_count - 1) > 0)
        {
          //Not loaded
          regs.stream1_count = 1;
          stream1_fileload(regs.stream1_count - 1);
        }
      }
      
      if (regs.stream1_loaded)
      {
        //Lock max value at 0x7F for bigger packets
        if (regs.stream1_queue >= 128)
          return 0x7F;
        else
          return regs.stream1_queue;
      }
      else
      {
        return 0;
      }
    }
    case 0x218b: {
      //Prefix Data Latch
      if (regs.pf_latch1_enable)
      {
        //Latch enabled
        if (regs.r2188 == 0 && regs.r2189 == 0)
        {
          //Time Channel, only one packet, both start and end
          regs.r218b = 0x90;
        }

        if(!Memory::debugger_access())
        {
          if (regs.stream1_loaded)
          {
            uint8 temp = 0;
            if (regs.stream1_first)
            {
              //First packet
              temp |= 0x10;
              regs.stream1_first = false;
            }

            regs.stream1_queue--;
            if (regs.stream1_queue == 0)
            {
              //Last packet
              temp |= 0x80;
            }
            regs.r218b = temp;
          }
        }

        regs.r218d |= regs.r218b;
        return regs.r218b;
      }
      else
      {
        //Latch not enabled
        return 0;
      }
    }
    case 0x218c: {
      //Data Latch
      if (regs.dt_latch1_enable)
      {
        if(!Memory::debugger_access())
        {
          if (regs.r2188 == 0 && regs.r2189 == 0)
          {
            //Return Time
            regs.r218c = get_time(false);
          }
          else if (regs.stream1_loaded)
          {
            //Get packet data
            regs.r218c = SAT1.read();
          }
        }
        return regs.r218c;
      }
      else
      {
        return 0;
      }
    }
    case 0x218d: {
      //Prefix Data OR Gate
      uint8 temp = regs.r218d;
      if(!Memory::debugger_access())
      {
        regs.r218d = 0;
      }
      return temp; 
    }

    //Stream 2
    case 0x218e: return regs.r218e; //Logical Channel 1 + Data Structure
    case 0x218f: return regs.r218f; //Logical Channel 2

    case 0x2190: {
      //Prefix Data Count
      if (!regs.pf_latch2_enable || !regs.dt_latch2_enable)
      {
        return 0;
      }

      if (regs.r218e == 0 && regs.r218f == 0)
      {
        //Time Channel, one packet
        return 0x01;
      }
      
      if(!Memory::debugger_access())
      {
        if (regs.stream2_queue <= 0)
        {
          //Queue is empty
          regs.stream2_count++;
          stream2_fileload(regs.stream2_count - 1);
        }
      
        if (!regs.stream2_loaded && (regs.stream2_count - 1) > 0)
        {
          //Not loaded
          regs.stream2_count = 1;
          stream2_fileload(regs.stream2_count - 1);
        }
      }
      
      if (regs.stream2_loaded)
      {
        if (regs.stream2_queue >= 128)
          return 0x7F;
        else
          return regs.stream2_queue;
      }
      else
      {
        return 0;
      }
    }
    case 0x2191: {
      //Prefix Data Latch
      if (regs.pf_latch2_enable)
      {
        //Latch enabled
        if (regs.r218e == 0 && regs.r218f == 0)
        {
          //Time Channel, only one packet, both start and end
          regs.r2191 = 0x90;
        }

        if(!Memory::debugger_access())
        {
          if (regs.stream2_loaded)
          {
            uint8 temp = 0;
            if (regs.stream2_first)
            {
              //First packet
              temp |= 0x10;
              regs.stream2_first = false;
            }

            regs.stream2_queue--;
            if (regs.stream2_queue == 0)
            {
              //Last packet
              temp |= 0x80;
            }
            regs.r2191 = temp;
          }
        }

        regs.r2193 |= regs.r2191;
        return regs.r2191;
      }
      else
      {
        //Latch not enabled
        return 0;
      }
    }

    case 0x2192: {
      //Data Latch
      if (regs.dt_latch2_enable)
      {
        if(!Memory::debugger_access())
        {
          if (regs.r218e == 0 && regs.r218f == 0)
          {
            //Return Time
            regs.r2192 = get_time(false);
          }
          else if (regs.stream2_loaded)
          {
            //Get packet data
            regs.r2192 = SAT1.read();
          }
        }

        return regs.r2192;
      }
      else
      {
        return 0;
      }
    }

    case 0x2193: {
      //Prefix Data OR
      uint8 temp = regs.r2193;
      if(!Memory::debugger_access())
      {
        regs.r2193 = 0;
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
  switch(addr) {
    //Stream 1
    case 0x2188: {
      //Logical Channel 1 + Data Structure
      if (regs.r2188 != data)
        regs.stream1_count = 0;
      regs.r2188 = data;
    } break;

    case 0x2189: {
      //Logical Channel 2 (6bit)
      if (regs.r2189 != (data & 0x3F))
        regs.stream1_count = 0;
      regs.r2189 = data & 0x3F;
    } break;

    case 0x218b: {
      //Prefix Data Latch
      regs.pf_latch1_enable = (data != 0);
    } break;

    case 0x218c: {
      //Data Latch
      if (regs.r2188 == 0 && regs.r2189 == 0 && !Memory::debugger_access())
      {
        //Hardcoded Time Channel
        get_time(true);
      }
      regs.dt_latch1_enable = (data != 0);
    } break;


    //Stream 2
    case 0x218e: {
      //Logical Channel 1 + Data Structure
      if (regs.r218e != data)
        regs.stream2_count = 0;
      regs.r218e = data;
    } break;

    case 0x218f: {
      //Logical Channel 2 (6bit)
      if (regs.r218f != (data & 0x3F))
        regs.stream2_count = 0;
      regs.r218f = data & 0x3F;
    } break;

    case 0x2191: {
      //Prefix Data Latch
      regs.pf_latch2_enable = (data != 0);
    } break;

    case 0x2192: {
      //Data Latch
      if (regs.r218e == 0 && regs.r218f == 0 && !Memory::debugger_access())
      {
        //Hardcoded Time Channel
        get_time(true);
      }
      regs.dt_latch2_enable = (data != 0);
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

