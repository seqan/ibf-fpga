#pragma once

namespace min_ibf_fpga::backend_sycl
{

template <typename constants, typename types>
struct minimizer_kernel
{
using Element = typename types::Element;
using Hash = typename types::Hash;
using Minimizer = typename types::Minimizer;
using QueryIndex = typename types::QueryIndex;
using MinimizerToIBFData = typename types::MinimizerToIBFData;

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

template <typename query_char_at_fn_t, typename on_minimizer_fn_t>
static void compute_minimizer(const QueryIndex querySize, query_char_at_fn_t && query_char_at_fn, on_minimizer_fn_t && on_minimizer_fn)
{
	//out << constants::initialization_iterations << " + " << (unsigned)querySize << " - " << WINDOW_SIZE << " + 1" << sycl::endl; // DEBUG

	const QueryIndex iterations =
		constants::initialization_iterations // Fill query and hash buffer initially
		+ querySize - constants::window_size + 1;

	char queryBuffer[constants::min_ibf_k] = {0};
	Hash hashBuffer[constants::number_of_kmers_per_window] = {0};

	// Set initial element's position to 0, so the first real element will never be skipped
	Minimizer lastMinimizer = {0, 0};

	for (QueryIndex iteration = 0; iteration <= iterations; iteration++)
	{
		// DEBUG
		//out << static_cast<unsigned>(iteration) << ": " << queryBuffer << " - ";
		//for (size_t i = 0; i < constants::min_ibf_k; i++)
		//	out << queryBuffer[i];
		//out << " - ";
		//for (size_t i = 0; i < constants::number_of_kmers_per_window; i++)
		//	out << static_cast<unsigned>(hashBuffer[i]) << ",";
		//out << sycl::endl;

		// Shift register: Query buffer
		#pragma unroll
		for (unsigned char i = 0; i < constants::min_ibf_k - 1; ++i)
			queryBuffer[i] = queryBuffer[i + 1];

		// Query as long as elements are left, then only do calculations (end phase)
		if (iteration < querySize)
			queryBuffer[constants::min_ibf_k - 1] = query_char_at_fn(iteration);

		// Shift register: hash buffer
		#pragma unroll
		for (ushort i = 0; i < constants::number_of_kmers_per_window - 1; ++i)
			hashBuffer[i] = hashBuffer[i + 1];

		hashBuffer[constants::number_of_kmers_per_window - 1] = extractHash(queryBuffer); // , out); // DEBUG

		const Minimizer minimizer = findMinimizer(hashBuffer);
		const Minimizer localLastMinimizer = lastMinimizer;

		if (iteration >= constants::initialization_iterations)
			// Update the lastMinimizer every time, because it is either new or the same anyways
			lastMinimizer = minimizer;

		const bool skipMinimizer = localLastMinimizer.position != 0
			&& localLastMinimizer.hash == minimizer.hash;
		const bool lastElement = iteration > iterations - 1;

		if (// Skip the first element (> instead of >=), as lastMinimizer is not initialized yet
			iteration > constants::initialization_iterations
			// Send the lastMinimizer when a new one is found or the last element is reached
			&& (!skipMinimizer || lastElement))
		{
			MinimizerToIBFData data;
			data.isLastElement = lastElement;
			data.hash = localLastMinimizer.hash;

			// DEBUG
			//out << "Found minimizer: " << static_cast<unsigned>(data.hash) << " " << data.isLastElement << sycl::endl; // DEBUG

			on_minimizer_fn(std::move(data));
		}
	}
}
};

} // namespace min_ibf_fpga::backend_sycl
