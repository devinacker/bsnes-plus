#ifdef DOS_CPP

#define CHIPS_IMPL
//#define CHIPS_ASSERT(...) // dummy
#include "fdd.h"
#include "fdd_dos.h"
#include "upd765.h"

DOSFloppy::DOSFloppy() {
  upd765_desc_t desc;
  desc.seektrack_cb = seek_track;
  desc.seeksector_cb = seek_sector;
  desc.read_cb = read_sector;
  desc.write_cb = write_sector;
  desc.trackinfo_cb = track_info;
  desc.driveinfo_cb = drive_info;
  desc.irq_cb = irq_set;
  desc.user_data = this;
  
  fdc = new upd765_t;
  fdd = new fdd_t[4];
  
  upd765_init(fdc, &desc);
}

DOSFloppy::~DOSFloppy() {
  delete fdc;
  delete[] fdd;
}

void DOSFloppy::reset() {

  upd765_reset(fdc);

  file discimage;
  for(int i; i < 4; i++){
    discimage.open(string(cartridge.basename(), "-floppy-", i+1, ".raw"), file::mode::writeread);
    if(fdd[i].has_disc){
      discimage.truncate(0);
      discimage.write(fdd[i].data, fdd[i].data_size);
    } else {
      fdd_init(&fdd[i]);
      if(discimage.size() > 0){
        fdd_dos_insert_empty(&fdd[i], discimage.size());
        discimage.read(fdd[i].data, discimage.size());
        printf("%d\n", discimage.size());
      } else {
        fdd_dos_insert_empty(&fdd[i], FDD_MAX_DISC_SIZE);
      }
    }
    discimage.close();
  }
}

int DOSFloppy::seek_track(int drive, int track, void* user_data) {
  return static_cast<DOSFloppy*>(user_data)->seek_track(drive, track);
}

int DOSFloppy::seek_track(int drive, int track) {
  if (fdd[drive].has_disc) {
    return fdd_seek_track(&fdd[drive], track);
  }
  return UPD765_RESULT_NOT_READY;
}

int DOSFloppy::seek_sector(int drive, upd765_sectorinfo_t* info, void* user_data) {
  return static_cast<DOSFloppy*>(user_data)->seek_sector(drive, info);
}

int DOSFloppy::seek_sector(int drive, upd765_sectorinfo_t* info) {
  if (fdd[drive].has_disc) {
    const uint8_t c = info->c;
    const uint8_t h = info->h;
    const uint8_t r = info->r;
    const uint8_t n = info->n;
    int res = fdd_seek_sector(&fdd[drive], c, h, r, n);
    if (res == UPD765_RESULT_SUCCESS) {
      const fdd_sector_t* sector = &fdd[drive].disc.tracks[h][fdd[drive].cur_track_index].sectors[fdd[drive].cur_sector_index];
      info->c = sector->info.upd765.c;
      info->h = sector->info.upd765.h;
      info->r = sector->info.upd765.r;
      info->n = sector->info.upd765.n;
      info->st1 = sector->info.upd765.st1;
      info->st2 = sector->info.upd765.st2;
    }
    return res;
  }
  return UPD765_RESULT_NOT_READY;
}

int DOSFloppy::read_sector(int drive, uint8_t h, void* user_data, uint8_t* data) {
  return static_cast<DOSFloppy*>(user_data)->read_sector(drive, h, data);
}

int DOSFloppy::read_sector(int drive, uint8_t h, uint8_t* data) {
  if (fdd[drive].has_disc) {
    return fdd_read(&fdd[drive], h, data);
  }
  return UPD765_RESULT_NOT_READY;
}

int DOSFloppy::write_sector(int drive, uint8_t h, void* user_data, uint8_t data) {
  return static_cast<DOSFloppy*>(user_data)->write_sector(drive, h, data);
}

int DOSFloppy::write_sector(int drive, uint8_t h, uint8_t data) {
  if (fdd[drive].has_disc) {
    return fdd_write(&fdd[drive], h, data);
  }
  return UPD765_RESULT_NOT_READY;
}

int DOSFloppy::track_info(int drive, int side, void* user_data, upd765_sectorinfo_t* info) {
  return static_cast<DOSFloppy*>(user_data)->track_info(drive, side, info);
}

int DOSFloppy::track_info(int drive, int side, upd765_sectorinfo_t* info) {
  CHIPS_ASSERT((side >= 0) && (side < 2));
  if (fdd[drive].has_disc && fdd[drive].motor_on) {
    // FIXME: this should be a fdd_ call
    info->physical_track = fdd[drive].cur_track_index;
    const fdd_sector_t* sector = &fdd[drive].disc.tracks[side][fdd[drive].cur_track_index].sectors[0];
    info->c = sector->info.upd765.c;
    info->h = sector->info.upd765.h;
    info->r = sector->info.upd765.r;
    info->n = sector->info.upd765.n;
    info->st1 = sector->info.upd765.st1;
    info->st2 = sector->info.upd765.st2;
    //printf("ok\n");
    return FDD_RESULT_SUCCESS;
  }
  return FDD_RESULT_NOT_READY;
}

void DOSFloppy::drive_info(int drive, void* user_data, upd765_driveinfo_t* info) {
  static_cast<DOSFloppy*>(user_data)->drive_info(drive, info);
}

void DOSFloppy::drive_info(int drive, upd765_driveinfo_t* info) {
  if (fdd[drive].has_disc) {
    info->physical_track = fdd[drive].cur_track_index;
    info->sides = fdd[drive].disc.num_sides;
    info->head = fdd[drive].cur_side;
    info->ready = fdd[drive].motor_on;
    info->write_protected = fdd[drive].disc.write_protected;
    info->fault = false;
  } else {
    info->physical_track = 0;
    info->sides = 2;
    info->head = 0;
    info->ready = true;
    info->write_protected = false;
    info->fault = false;
  }
}

void DOSFloppy::irq_set(void* user_data, bool status) {
  static_cast<DOSFloppy*>(user_data)->irq_set(status);
}

void DOSFloppy::irq_set(bool status) {
  irq_pending = status;
  dos.irq_process();
}

uint8 DOSFloppy::read(bool addr) {
  uint64_t pins = UPD765_CS | UPD765_RD;
  if (addr) pins |= UPD765_A0;
  
  pins = upd765_iorq(fdc, pins);
  //printf("[%06x] read 5f2%d > %02x (irq: %x, phase: %d)\n", cpu.regs.pc, addr, UPD765_GET_DATA(pins), irq_pending, fdc->phase);
  return UPD765_GET_DATA(pins);
}

void DOSFloppy::write(bool addr, uint8 data) {
  uint64_t pins = UPD765_CS | UPD765_WR;
  if (addr) pins |= UPD765_A0;
  UPD765_SET_DATA(pins, data);
  
  upd765_iorq(fdc, pins);
  //printf("[%06x] write 5f2%d > %02x (irq: %x, phase: %d)\n", cpu.regs.pc, addr, data, irq_pending, fdc->phase);
}

void DOSFloppy::terminate() {
  upd765_iorq(fdc, UPD765_CS | UPD765_TC);
}

bool DOSFloppy::irq_status() const {
  return irq_pending;
}

#endif
