#ifdef DOS_CPP

#define CHIPS_IMPL
#define CHIPS_ASSERT(...) // dummy
#include "upd765.h"

DOSFloppy::DOSFloppy() {
  upd = new upd765_t;

  upd765_desc_t desc;
  desc.seektrack_cb = seek_track;
  desc.seeksector_cb = seek_sector;
  desc.read_cb = read_sector;
  desc.trackinfo_cb = track_info;
  desc.driveinfo_cb = drive_info;
  desc.user_data = this;
  upd765_init(upd, &desc);

  reset();
}

DOSFloppy::~DOSFloppy() {
  delete upd;
}

void DOSFloppy::reset() {
  upd765_reset(upd);
}

int DOSFloppy::seek_track(int drive, int track, void* user_data) {
  return static_cast<DOSFloppy*>(user_data)->seek_track(drive, track);
}

int DOSFloppy::seek_track(int drive, int track) {
  return UPD765_RESULT_NOT_READY; // TODO
}

int DOSFloppy::seek_sector(int drive, upd765_sectorinfo_t* info, void* user_data) {
  return static_cast<DOSFloppy*>(user_data)->seek_sector(drive, info);
}

int DOSFloppy::seek_sector(int drive, upd765_sectorinfo_t* info) {
  return UPD765_RESULT_NOT_READY; // TODO
}

int DOSFloppy::read_sector(int drive, uint8_t h, void* user_data, uint8_t* data) {
  return static_cast<DOSFloppy*>(user_data)->read_sector(drive, h, data);
}

int DOSFloppy::read_sector(int drive, uint8_t h, uint8_t* data) {
  return UPD765_RESULT_NOT_READY; // TODO
}

int DOSFloppy::track_info(int drive, int side, void* user_data, upd765_sectorinfo_t* info) {
  return static_cast<DOSFloppy*>(user_data)->track_info(drive, side, info);
}

int DOSFloppy::track_info(int drive, int side, upd765_sectorinfo_t* info) {
  return UPD765_RESULT_NOT_READY; // TODO
}

void DOSFloppy::drive_info(int drive, void* user_data, upd765_driveinfo_t* info) {
  static_cast<DOSFloppy*>(user_data)->drive_info(drive, info);
}

void DOSFloppy::drive_info(int drive, upd765_driveinfo_t* info) {
  info->physical_track = 0; // TODO
  info->sides = 2;
  info->head = 0; // TODO
  info->ready = false; // TODO
  info->write_protected = true; // TODO
  info->fault = true; // TODO
}

uint8 DOSFloppy::read(bool addr) {
  uint64_t pins = UPD765_CS | UPD765_RD;
  if (addr) pins |= UPD765_A0;
  
  pins = upd765_iorq(upd, pins);
  return UPD765_GET_DATA(pins);
}

void DOSFloppy::write(bool addr, uint8 data) {
  uint64_t pins = UPD765_CS | UPD765_WR;
  if (addr) pins |= UPD765_A0;
  UPD765_SET_DATA(pins, data);
  
  upd765_iorq(upd, pins);
}

bool DOSFloppy::irq_status() const {
  return false; // TODO
}

#endif
