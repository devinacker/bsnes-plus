Random random;

void Random::seed(unsigned seed) {
  _random.seed = seed;
}

unsigned Random::operator()(unsigned result) {
  return config().random ? _random() : result;
}

void Random::serialize(serializer& s) {
  s.integer(_random.seed);
}
