#ifdef BSX_CPP

void BSXBase::serialize(serializer &s) {
	Processor::serialize(s);

	for (auto &stream : regs.stream) {
		s.integer(stream.offset);
		s.integer(stream.loaded_channel);
		s.integer(stream.loaded_count);
		
		if (s.mode() == serializer::Load && stream.loaded_channel > 0) {
			stream.channel = stream.loaded_channel;
			stream.count = stream.loaded_count;
			stream_fileload(stream);
			stream.packets.seek(stream.offset);
		}
		
		s.integer(stream.channel);
		s.integer(stream.prefix);
		s.integer(stream.data);
		s.integer(stream.status);
		
		s.integer(stream.pf_latch);
		s.integer(stream.dt_latch);
		s.integer(stream.count);
		s.integer(stream.first);
		s.integer(stream.queue);
		s.integer(stream.pf_queue);
		s.integer(stream.dt_queue);
		
		s.integer(stream.time.tm_hour);
		s.integer(stream.time.tm_min);
		s.integer(stream.time.tm_sec);
		s.integer(stream.time.tm_wday);
		s.integer(stream.time.tm_mday);
		s.integer(stream.time.tm_mon);
	}
	
	s.integer(regs.r2194);
	s.integer(regs.r2195);
	s.integer(regs.r2196);
	s.integer(regs.r2197);
	s.integer(regs.r2198);
	s.integer(regs.r2199);
	s.integer(regs.r219a);
	
	s.integer(local_time);
	s.integer(custom_time);
	
	// use an offset to start_time so that custom times stay continuous
	time_t now, elapsed;
	time(&now);
	if (s.mode() == serializer::Load) {
		s.integer(elapsed);
		start_time = now - elapsed;
	} else {
		elapsed = now - start_time;
		s.integer(elapsed);
	}
}

void BSXCart::serialize(serializer &s) {
	s.array(regs.r);
	s.array(regs.rtemp);
	s.integer(regs.irq);
	s.integer(regs.irq_en);
	s.integer(regs.dirty);
	
	if (s.mode() == serializer::Load)
		update_memory_map();
}

void BSXFlash::serialize(serializer &s) {
	s.integer(regs.command);
	s.integer(regs.csr);
	s.integer(regs.esr);
	s.integer(regs.vendor_info);
	s.integer(regs.writebyte);
	
	if (s.mode() == serializer::Load)
		memory::bsxpack.write_protect(!regs.writebyte);
}

#endif
