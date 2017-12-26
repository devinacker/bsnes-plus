Reader reader;

const char* Reader::direct_supported() {
  return "";
}

bool Reader::direct_load(string &filename, uint8_t *&data, unsigned &size) {
  if(file::exists(filename) == false) return false;

  file fp;
  if(fp.open(filename, file::mode::read) == false) return false;

  data = new uint8_t[size = fp.size()];
  fp.read(data, size);
  fp.close();

  //remove copier header, if it exists
  if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);

  return true;
}

Reader::Reader() {
  if(open("snesreader")) {
    supported = sym("snesreader_supported");
    load = sym("snesreader_load");
  }

  if(!supported || !load) {
    supported = { &Reader::direct_supported, this };
    load = { &Reader::direct_load, this };
  }

  compressionList = supported();
  if(compressionList.length() > 0) compressionList = string() << " " << compressionList;

  if(opened()) {
    extensionList = string()
    << " *.swc *.fig" // no need of snesreader to read *.smc? see line 17 and 18 for removing copier header  by komicakomica
    << " *.ufo *.gd3 *.gd7 *.dx2 *.mgd *.mgh"
    << " *.048 *.058 *.068 *.078 *.bin"
    << " *.usa *.eur *.jap *.aus *.bsx";
  }
}
