#pragma once
/*#
    # fdd_dos.h

    Extension functions for fdd.h to load Amstrad dos emulator disk
    image formats into fdd_t.

    Include fdd_dos.h right after fdd.h, both for the declaration and
    implementation.

    ## Functions

    ~~~C
    bool fdd_dos_insert_img(fdd_t* fdd, const uint8_t* data, int data_size)
    ~~~
        'Inserts' a dos .img disk image into the floppy drive.

        fdd         - pointer to an initialized fdd_t instance
        data        - pointer to the .img image data in memory
        data_size   - size in bytes of the image data

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#ifdef __cplusplus
extern "C" {
#endif

/* load Amstrad dos .img file format */
bool fdd_dos_insert_img(fdd_t* fdd, const uint8_t* data, int data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*--- IMPLEMENTATION ---------------------------------------------------------*/
#ifdef CHIPS_IMPL

/*
    load an DOS disc image (normal or extended file format)

    https://thestarman.pcministry.com/asm/mbr/DOS50FDB.htm
*/
typedef struct _fdd_dos_img_header {
    uint16_t code_ptr;
    uint8_t nop;      /* MV - dosEMU.... */
    uint8_t magic[8];      /* MSDOS5.0 */
    //bpb
    uint16_t sector_size;
    uint8_t sectors_cluster;
    uint16_t reserved_sector;
    uint8_t fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t mdb; // media descriptor byte
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_sides; //heads
    uint8_t zeroes[10];
    uint8_t ext_info;
    uint8_t serial[4];
    uint8_t volume_label[11];
    uint8_t filesys_id[8];
    //uint8_t code[140]; unecessary
    //uint8_t error_msg[140]; unecessary

} _fdd_dos_img_header;

typedef struct _fdd_dos_img_track_info {
    uint8_t magic[12];      /* Track-Info\r\n */
    uint8_t unused_0[4];
    uint8_t track_number;
    uint8_t side_number;
    uint8_t unused_1[2];
    uint8_t sector_size;
    uint8_t num_sectors;
    uint8_t gap_length;
    uint8_t filler_byte;
} _fdd_dos_img_track_info;

typedef struct _fdd_dos_img_sector_info {
    uint8_t track;
    uint8_t side;
    uint8_t sector_id;
    uint8_t sector_size;    /* size_in_bytes = 0x80<<sector_size */
    uint8_t st1;
    uint8_t st2;
    uint8_t ext[2];         /* in extended disk format, actual sector data size in bytes */
} _fdd_dos_img_sector_info;

/* parse a standard .img image */
static bool _fdd_dos_parse_img(fdd_t* fdd, const uint8_t* data, int data_size) {
    CHIPS_ASSERT(fdd);
    const _fdd_dos_img_header* hdr = (_fdd_dos_img_header*) data;
    uint8_t num_tracks = hdr->total_sectors / hdr->sectors_per_track;
    uint8_t track_size = hdr->sector_size * hdr->sectors_per_track;
    if (hdr->num_sides > 2) {
        return false;
        printf("sides %d\n", hdr->num_sides);
    }
    if (num_tracks > FDD_MAX_TRACKS) {
        printf("track %d\n", num_tracks);
        return false;
    }
    /* copy the data blob to the local buffer */
    fdd->data_size = data_size;
    memcpy(fdd->data, data, fdd->data_size);

    /* setup the disc structure */
    fdd_disc_t* disc = &fdd->disc;
    disc->formatted = true;
    disc->num_sides = hdr->num_sides;
    disc->num_tracks = num_tracks;
    int data_offset = 0;//sizeof(_fdd_dos_img_header);
    for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
        for (int side_index = 0; side_index < disc->num_sides; side_index++) {
            fdd_track_t* track = &disc->tracks[side_index][track_index];
            track->data_offset = data_offset;
            track->data_size = track_size;
            track->num_sectors = hdr->sectors_per_track;

            int sector_data_offset = data_offset;
            for (int sector_index = 0; sector_index < track->num_sectors; sector_index++) {
                fdd_sector_t* sector = &track->sectors[sector_index];
                sector->info.upd765.c = track_index;
                sector->info.upd765.h = side_index;
                sector->info.upd765.r = sector_index;
                sector->info.upd765.n = hdr->sector_size;
                sector->info.upd765.st1 = 0;
                sector->info.upd765.st2 = 0;
                sector->data_offset = sector_data_offset;
                sector->data_size = hdr->sector_size;
                sector_data_offset += hdr->sector_size;
            }
            data_offset += track_size;
            CHIPS_ASSERT(data_offset == sector_data_offset);
        }
        /*else {
            /* unformatted / non-existing track 
            track->data_offset = 0;
            track->data_size = 0;
            track->num_sectors = 0;
        }*/
    }
    fdd->has_disc = true;
    return true;
}

bool fdd_dos_insert_img(fdd_t* fdd, const uint8_t* data, int data_size) {

        printf("wee basdasdasad\n");
    CHIPS_ASSERT(fdd);
    //CHIPS_ASSERT(sizeof(_fdd_dos_img_header) == 256);
    //CHIPS_ASSERT(sizeof(_fdd_dos_img_track_info) == 24);
    //CHIPS_ASSERT(sizeof(_fdd_dos_img_sector_info) == 8);
    CHIPS_ASSERT(data && (data_size > 0));
    if (fdd->has_disc) {
        fdd_eject_disc(fdd);
    }

    /* check if the header is valid */
    if (data_size > FDD_MAX_DISC_SIZE) {
        printf("wee pls %d\n", FDD_MAX_DISC_SIZE * 2);
        return false;
    }
    if (data_size <= (int)sizeof(_fdd_dos_img_header)) {
        printf("wee bad\n");
        return false;
    }
    const _fdd_dos_img_header* hdr = (_fdd_dos_img_header*) data;

    if (0 == memcmp(hdr->magic, "MSDOS5.0", 8)) {
        printf("wee good\n");
        return false;
        if (_fdd_dos_parse_img(fdd, data, data_size)) {
            return true;
        }
    }
    printf("wrong format: %c\n", hdr->magic);
    fdd_eject_disc(fdd);
    return false;
}


/* parse a standard .img image */
static bool fdd_dos_insert_empty(fdd_t* fdd, int data_size) {
    CHIPS_ASSERT(fdd);

    fdd->data_size = data_size;

    fdd_disc_t* disc = &fdd->disc;
    disc->formatted = true;
    disc->num_sides = FDD_MAX_SIDES;
    disc->num_tracks = FDD_MAX_TRACKS;
    int data_offset = 0;
    for (int track_index = 0; track_index < disc->num_tracks; track_index++) {
        for (int side_index = 0; side_index < disc->num_sides; side_index++) {
            fdd_track_t* track = &disc->tracks[side_index][track_index];
            track->data_offset = data_offset;
            track->data_size = FDD_MAX_TRACK_SIZE;
            track->num_sectors = FDD_MAX_SECTORS;

            int sector_data_offset = data_offset;
            for (int sector_index = 0; sector_index < track->num_sectors; sector_index++) {
                fdd_sector_t* sector = &track->sectors[sector_index];
                sector->info.upd765.c = track_index;
                sector->info.upd765.h = side_index;
                sector->info.upd765.r = sector_index;
                sector->info.upd765.n = 0xFF;
                sector->info.upd765.st1 = 0;
                sector->info.upd765.st2 = 0;
                sector->data_offset = sector_data_offset;
                sector->data_size = 0xFF;
                sector_data_offset += 0xFF;
            }
            data_offset += FDD_MAX_TRACK_SIZE;
            CHIPS_ASSERT(data_offset == sector_data_offset);
        }
        /*else {
            /* unformatted / non-existing track 
            track->data_offset = 0;
            track->data_size = 0;
            track->num_sectors = 0;
        }*/
    }
    fdd->has_disc = true;
    fdd->motor_on = true;
    return true;
}

#endif /* CHIPS_IMPL */
