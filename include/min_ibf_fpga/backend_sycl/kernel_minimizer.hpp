#include <sycl/sycl.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include "commen.hpp"
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

namespace min_ibf_fpga::backend_sycl
{

Element inline translateCharacterToElement(const char character)
{
	return character == 'A'? 0
		: character == 'C'? 1
		: character == 'G'? 2
		: 3;
}

Hash inline extractHash(const char* buffer) //, sycl::stream out) // DEBUG
{
	Hash kmer = 0;
	Hash kmerComplement = 0;

	#pragma unroll
	for (unsigned char elementIndex = 0; elementIndex < MIN_IBF_K; ++elementIndex)
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

		kmer           |= (Hash)(value) << (2 * (MIN_IBF_K - 1) - elementIndex * 2);
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

	kmer ^= (Hash)MINIMIZER_SEED_ADJUSTED;
	kmerComplement ^= (Hash)MINIMIZER_SEED_ADJUSTED;

	return kmer < kmerComplement? kmer : kmerComplement;
}

Minimizer inline findMinimizer(const Hash* hashBuffer)
{
	Minimizer minimizer = {~(Hash)0, 0};

	#pragma unroll
	for (unsigned char kmerIndex = 0; kmerIndex < NUMBER_OF_KMERS_PER_WINDOW; ++kmerIndex)
	{
		const Minimizer current = {hashBuffer[kmerIndex], kmerIndex};

		if (current.hash < minimizer.hash)
			minimizer = current;
	}

	return minimizer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Minimizer
////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class MinimizerKernel;
class PipeToHostKernel;

void RunMinimizerKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const HostSizeType queriesOffset,
	sycl::buffer<HostSizeType, 1>& querySizes_buffer,
	const HostSizeType querySizesOffset,
	const HostSizeType numberOfQueries,
	sycl::buffer<MinimizerToIBFData, 1>& minimizerToIbf_buffer)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor queries(queries_buffer, handler, sycl::read_only);
			sycl::accessor querySizes(querySizes_buffer, handler, sycl::read_only);

			sycl::stream out(65536, 256, handler); // DEBUG
			handler.single_task<MinimizerKernel>([=]() [[intel::kernel_args_restrict]]
			{
				QueryIndex queryOffset = queriesOffset;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					const QueryIndex querySize = querySizes[static_cast<size_t>(querySizesOffset) + static_cast<size_t>(queryIndex)]; //__prefetching_load(&

					const QueryIndex localQueryOffset = queryOffset;
					queryOffset += querySize;

					//out << INITIALIZATION_ITERATIONS << " + " << (unsigned)querySize << " - " << WINDOW_SIZE << " + 1" << sycl::endl; // DEBUG

					const QueryIndex iterations =
						INITIALIZATION_ITERATIONS // Fill query and hash buffer initially
						+ querySize - WINDOW_SIZE + 1;

					char queryBuffer[MIN_IBF_K] = {0};
					Hash hashBuffer[NUMBER_OF_KMERS_PER_WINDOW] = {0};

					// Set initial element's position to 0, so the first real element will never be skipped
					Minimizer lastMinimizer = {0, 0};

					for (QueryIndex iteration = 0; iteration <= iterations; iteration++)
					{
						// DEBUG
						//out << static_cast<unsigned>(iteration) << ": " << queryBuffer << " - ";
						//for (size_t i = 0; i < MIN_IBF_K; i++)
						//	out << queryBuffer[i];
						//out << " - ";
						//for (size_t i = 0; i < NUMBER_OF_KMERS_PER_WINDOW; i++)
						//	out << static_cast<unsigned>(hashBuffer[i]) << ",";
						//out << sycl::endl;

						// Shift register: Query buffer
						#pragma unroll
						for (unsigned char i = 0; i < MIN_IBF_K - 1; ++i)
							queryBuffer[i] = queryBuffer[i + 1];

						// Query as long as elements are left, then only do calculations (end phase)
						if (iteration < querySize)
							queryBuffer[MIN_IBF_K - 1] = queries[static_cast<size_t>(localQueryOffset + iteration)]; //__prefetching_load(&

						// Shift register: hash buffer
						#pragma unroll
						for (ushort i = 0; i < NUMBER_OF_KMERS_PER_WINDOW - 1; ++i)
							hashBuffer[i] = hashBuffer[i + 1];

						hashBuffer[NUMBER_OF_KMERS_PER_WINDOW - 1] = extractHash(queryBuffer); // , out); // DEBUG

						const Minimizer minimizer = findMinimizer(hashBuffer);
						const Minimizer localLastMinimizer = lastMinimizer;

						if (iteration >= INITIALIZATION_ITERATIONS)
							// Update the lastMinimizer every time, because it is either new or the same anyways
							lastMinimizer = minimizer;

						const bool skipMinimizer = localLastMinimizer.position != 0
							&& localLastMinimizer.hash == minimizer.hash;
						const bool lastElement = iteration > iterations - 1;

						if (// Skip the first element (> instead of >=), as lastMinimizer is not initialized yet
							iteration > INITIALIZATION_ITERATIONS
							// Send the lastMinimizer when a new one is found or the last element is reached
							&& (!skipMinimizer || lastElement))
						{
							MinimizerToIBFData data;
							data.isLastElement = lastElement;
							data.hash = localLastMinimizer.hash;

							// DEBUG
							//out << "Found minimizer: " << static_cast<unsigned>(data.hash) << " " << data.isLastElement << sycl::endl; // DEBUG

							MinimizerToIBFPipes::PipeAt<id>::write(data);
						}
					}
				}
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor minimizerToIbf(minimizerToIbf_buffer, handler, sycl::write_only);

			handler.single_task<PipeToHostKernel>([=]() [[intel::kernel_args_restrict]]
			{
				size_t numberofMinimizersFound = 0;
				MinimizerToIBFData data;
				do
				{
					data = MinimizerToIBFPipes::PipeAt<id>::read();
					minimizerToIbf[numberofMinimizersFound] = data;
					numberofMinimizersFound++;
				}
				while(!data.isLastElement);
			});
		});
	});
}

} // namespace min_ibf_fpga::backend_sycl
