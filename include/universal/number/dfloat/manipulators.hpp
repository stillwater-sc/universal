//


namespace sw { namespace universal {

// Generate a type tag for this dfloat
template<unsigned ndigits, unsigned es, typename bt>
std::string type_tag(const dfloat<ndigits, es, bt>& = {}) {
	std::stringstream s;
	s << "dfloat<"
		<< std::setw(3) << ndigits << ", "
		<< std::setw(3) << es << ", "
		<< typeid(bt).name() << ">";
	return s.str();
}


}}; // namespace sw::universal
