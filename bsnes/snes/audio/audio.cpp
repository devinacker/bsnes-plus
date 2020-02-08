#ifdef SYSTEM_CPP

Audio audio;

Stream::Stream() {
  audio_init();
}

void Stream::audio_init() {
  stream_.rdoffset = 0;
  stream_.wroffset = 0;
  stream_.length = 0;

  stream_.r_sum_l = stream_.r_sum_r = 0;
  stream_.r_step = 1.0;
  stream_.r_frac = 0;
}

bool Stream::has_sample() {
  return stream_.length > 0;
}

uint32 Stream::get_sample() {
  uint32 sample_ = stream_.buffer[stream_.rdoffset];
  stream_.rdoffset = (stream_.rdoffset + 1) & 32767;
  stream_.length--;
  return sample_;
}

void Stream::audio_frequency(double input_frequency) {
  double output_frequency;
  output_frequency = system.apu_frequency() / 768.0;
  stream_.r_step = input_frequency / output_frequency;
  stream_.r_frac = 0;
}

void Stream::sample(int16 left, int16 right) {
  if(dsp.mute()) left = 0, right = 0;

  if(stream_.r_frac >= 1.0) {
    stream_.r_frac -= 1.0;
    stream_.r_sum_l += left;
    stream_.r_sum_r += right;
    return;
  }

  stream_.r_sum_l += left  * stream_.r_frac;
  stream_.r_sum_r += right * stream_.r_frac;

  uint16 output_left  = sclamp<16>(int(stream_.r_sum_l / stream_.r_step));
  uint16 output_right = sclamp<16>(int(stream_.r_sum_r / stream_.r_step));

  double first = 1.0 - stream_.r_frac;
  stream_.r_sum_l = left  * first;
  stream_.r_sum_r = right * first;
  stream_.r_frac = stream_.r_step - first;

  stream_.buffer[stream_.wroffset] = (output_left << 0) + (output_right << 16);
  stream_.wroffset = (stream_.wroffset + 1) & 32767;
  stream_.length = (stream_.length + 1) & 32767;
  audio.flush();
}

void Audio::init() {
  streams.reset();

  dsp_rdoffset = 0;
  dsp_wroffset = 0;
  dsp_length = 0;
}

void Audio::add_stream(Stream* stream) {
  dsp_rdoffset = 0;
  dsp_wroffset = 0;
  dsp_length = 0;

  stream->audio_init();
  streams.append(stream);
}
  
void Audio::sample(int16 left, int16 right) {
  if(!streams.size()) {
    system.interface->audio_sample(left, right);
  } else {
    dsp_buffer[dsp_wroffset] = ((uint16)left << 0) + ((uint16)right << 16);
    dsp_wroffset = (dsp_wroffset + 1) & 32767;
    dsp_length = (dsp_length + 1) & 32767;
    flush();
  }
}

void Audio::flush() {
  while(streams.size() && dsp_length) {
    for(unsigned i = 0; i < streams.size(); i++) {
      if (!streams[i]->has_sample()) return;
    }

    uint32 dsp_sample = dsp_buffer[dsp_rdoffset];
    dsp_rdoffset = (dsp_rdoffset + 1) & 32767;
    dsp_length--;

    int left  = (int16)(dsp_sample >>  0);
    int right = (int16)(dsp_sample >> 16);

    for(unsigned i = 0; i < streams.size(); i++) {
      uint32 sample = streams[i]->get_sample();
      left  += (int16)(sample >> 0);
      right += (int16)(sample >> 16);
    }

    system.interface->audio_sample(
      sclamp<16>(left / 2),
      sclamp<16>(right / 2)
    );
  }
}

#endif
