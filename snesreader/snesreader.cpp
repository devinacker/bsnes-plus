#include "snesreader.hpp"

#if defined(_WIN32)
  #define bsnesexport __declspec(dllexport)
#else
  #define bsnesexport
#endif

#include "fex/fex.h"
#include "libjma/jma.h"
extern "C" char* uncompressStream(int, int);  //micro-bunzip

#define QT_CORE_LIB
#include <QtWidgets>

#include <nall/file.hpp>
#include <nall/string.hpp>
using namespace nall;

bsnesexport const char* snesreader_supported() {
  return "*.zip *.z *.7z *.gz *.bz2 *.jma";
}

bool snesreader_load_normal(const char *filename, uint8_t *&data, unsigned &size) {
  file fp;
  if(fp.open(filename, file::mode_read) == false) return false;
  size = fp.size();
  data = new uint8_t[size];
  fp.read(data, size);
  fp.close();
  return true;
}

#include "filechooser.cpp"

bool snesreader_load_fex(string &filename, uint8_t *&data, unsigned &size) {
  fex_t *fex;
  fex_open(&fex, filename);
  if(fex_done(fex)) { fex_close(fex); return false; }

  if(!fileChooser) fileChooser = new FileChooser;
  fileChooser->list.reset();

  while(fex_done(fex) == false) {
    fex_stat(fex);
    const char *name = fex_name(fex);
    //only add valid ROM extensions to list (ignore text files, save RAM files, etc)
    if(striend(name, ".sfc") || striend(name, ".smc")
    || striend(name, ".swc") || striend(name, ".fig")
    || striend(name, ".bs")  || striend(name, ".st")
    || striend(name, ".gb")  || striend(name, ".sgb") || striend(name, ".gbc")
    || striend(filename, ".gz")  //GZip files only contain a single file
    ) {
      fileChooser->list[fileChooser->list.size()] = name;
    }
    fex_next(fex);
  }

  string name = fileChooser->exec();
  if(name == "") { fex_close(fex); return false; }

  fex_rewind(fex);
  while(fex_done(fex) == false) {
    fex_stat(fex);
    if(name == fex_name(fex)) {
      size = fex_size(fex);
      data = new uint8_t[size];
      fex_read(fex, data, size);
      fex_close(fex);

      if(fileChooser->list.size() > 1) {
        strtr(name, "\\", "/");
        strtr(filename, "\\", "/");

        //retain only path from filename, "/foo/bar.7z" -> "/foo/"
        for(signed i = filename.length() - 1; i >= 0; i--) {
          if(filename[i] == '/') {
            filename[i + 1] = 0;
            break;
          }
        }

        //append only filename from archive, "foo/bar.sfc" -> "bar.sfc"
        lstring part;
        part.split("/", name);
        filename = string() << filename << part[part.size() - 1];
      }

      return true;
    }
    fex_next(fex);
  }

  fex_close(fex);
  return false;
}

bool snesreader_load_bz2(const char *filename, uint8_t *&data, unsigned &size) {
  //TODO: need a way to get the size of a bzip2 file, so we can pre-allocate
  //a buffer to decompress into memory. for now, use a temporary file.

  string name = "/tmp/.bz2_temporary_decompression_object";
  FILE *wr;
  wr = fopen_utf8(name, "wb");
  if(!wr) {
    //try the local directory
    name = ".bz2_temporary_decompression_object";
    wr = fopen_utf8(name, "wb");
    //can't get write access, so give up
    if(!wr) return false;
  }

  FILE *fp = fopen_utf8(filename, "rb");
  uncompressStream(fileno(fp), fileno(wr));
  fclose(fp);
  fclose(wr);

  bool success = snesreader_load_normal(name, data, size);
  unlink(name);
  return success;
}

bool snesreader_load_jma(const char *filename, uint8_t *&data, unsigned &size) {
  try {
    JMA::jma_open JMAFile(filename);
    std::string name;

    std::vector<JMA::jma_public_file_info> file_info = JMAFile.get_files_info();
    for(std::vector<JMA::jma_public_file_info>::iterator i = file_info.begin(); i != file_info.end(); i++) {
      name = i->name;
      size = i->size;
      break;
    }

    data = new uint8_t[size];
    JMAFile.extract_file(name, data);
    return true;
  } catch(JMA::jma_errors) {
    return false;
  }
}

bsnesexport bool snesreader_load(string &filename, uint8_t *&data, unsigned &size) {
  if(file::exists(filename) == false) return false;

  bool success = false;
  if(striend(filename, ".zip")
  || striend(filename, ".z")
  || striend(filename, ".7z")
  || striend(filename, ".gz")) {
    success = snesreader_load_fex(filename, data, size);
  } else if(striend(filename, ".bz2")) {
    success = snesreader_load_bz2(filename, data, size);
  } else if(striend(filename, ".jma")) {
    success = snesreader_load_jma(filename, data, size);
  } else {
    success = snesreader_load_normal(filename, data, size);
  }

  return success;
}
