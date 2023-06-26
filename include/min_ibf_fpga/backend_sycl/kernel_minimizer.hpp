#pragma once

namespace min_ibf_fpga::backend_sycl
{

template <typename constants, typename types>
struct minimizer_kernel
{
using Element = typename types::Element;
using Hash = typename types::Hash;
using Minimizer = typename types::Minimizer;

static Element inline translateCharacterToElement(const char character)
{
	return character == 'A'? 0
		: character == 'C'? 1
		: character == 'G'? 2
		: 3;
}

static Hash inline extractHash(const char* buffer) //, sycl::stream out) // DEBUG
{
	Hash kmer = 0;
	Hash kmerComplement = 0;

	#pragma unroll
	for (unsigned char elementIndex = 0; elementIndex < constants::min_ibf_k; ++elementIndex)
	{
		const Element value = translateCharacterToElement(buffer[elementIndex]);

		//out << elementIndex << ": " << static_cast<unsigned>(kmer) << " - " << static_cast<unsigned>(kmerComplement) << sycl::endl;
		//	out << elementIndex << ": ";
		//for (unsigned i = 0; i < 2*MIN_IBF_K; i++)
		//	out << (unsigned)kmer[i];
		//out << " - ";
		//for (unsigned i = 0; i < 2*MIN_IBF_K; i++)
		//	out << (unsigned)kmerComplement[i];
		//out << sycl::endl;

		//out << "char " << static_cast<unsigned>(buffer[elementIndex]) << " value " << static_cast<unsigned>(value) << " ~value " << static_cast<unsigned>((Element)~value) << sycl::endl;

		kmer           |= (Hash)(value) << (2 * (constants::min_ibf_k - 1) - elementIndex * 2);
		kmerComplement |= (Hash)((Element)~value) << elementIndex * 2;
	}

	//out << "Extracted Hash " << static_cast<unsigned>(kmer) << " Complement " << static_cast<unsigned>(kmerComplement) << sycl::endl;
	//out << "XH ";
	//for (unsigned i = 0; i < 2*MIN_IBF_K; i++)
	//	out << (unsigned)kmer[i];
	//out << " C ";
	//for (unsigned i = 0; i < 2*MIN_IBF_K; i++)
	//	out << (unsigned)kmerComplement[i];
	//out << sycl::endl;

	kmer ^= (Hash)constants::minimizer_seed_adjusted;
	kmerComplement ^= (Hash)constants::minimizer_seed_adjusted;

	return kmer < kmerComplement? kmer : kmerComplement;
}

static Minimizer inline findMinimizer(const Hash* hashBuffer)
{
	Minimizer minimizer = {~(Hash)0, 0};

	#pragma unroll
	for (unsigned char kmerIndex = 0; kmerIndex < constants::number_of_kmers_per_window; ++kmerIndex)
	{
		const Minimizer current = {hashBuffer[kmerIndex], kmerIndex};

		if (current.hash < minimizer.hash)
			minimizer = current;
	}

	return minimizer;
}
};

} // namespace min_ibf_fpga::backend_sycl
