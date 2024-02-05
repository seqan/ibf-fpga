			// sycl::stream out(65536, 256, handler); // DEBUG
			handler.single_task<IbfKernel<id>>([=]() [[intel::kernel_args_restrict]]
			{
				sycl::ext::intel::device_ptr<const HostSizeType> thresholds_ptr_casted(thresholds_ptr);
				sycl::ext::intel::device_ptr<const Chunk> ibfData_ptr_casted(ibfData_ptr);

				HostSizeType thresholds[THRESHOLDS_CACHE_SIZE];

				const HostSizeType thresholdsMaxIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

				for (ushort i = 0; i <= thresholdsMaxIndex; i++)
				{
					thresholds[i] = thresholds_ptr_casted[i];
				}

				for (QueryIndex queryIndex = 0; queryIndex < localNumberOfQueries; queryIndex++)
				{
					[[intel::fpga_register]] Counter counters[CHUNKS][CHUNK_BITS];

					bool countersInitialized = 0;

					QueryIndex numberOfHashes = 1;

					MinimizerToIBFData data;

					do
					{
						data = MinimizerToIBFPipes::PipeAt<id>::read();

						const QueryIndex localNumberOfHashes = numberOfHashes++;

						Counter threshold = 0;

						if (data.isLastElement)
						{
							threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
							// out << "threshold: " << threshold << sycl::endl; // DEBUG
						}

						HostSizeType binOffsets[HASH_COUNT];

						// out << "binOffsets: "; // DEBUG

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
						{
							binOffsets[seedIndex] = calculateBinIndex(data.hash, seedIndex, hashShift, binSize) * CHUNKS_PER_BIN;
							// out << (size_t)binOffsets[seedIndex] << " "; // DEBUG
						}

						// out << sycl::endl; // DEBUG

						for (unsigned char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
						{
							[[intel::fpga_register]] Chunk bitvector = ~(Chunk)0;
							[[intel::fpga_register]] Chunk localResult = 0;

							// Unroll: Burst-coalesced over chunks per seed
							#pragma unroll
							for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
							{
								bitvector &= //__burst_coalesced_cached_load(
									/*&*/ibfData_ptr_casted[static_cast<size_t>(binOffsets[seedIndex]) + chunkIndex];//,
									//1048576); // 1 MiB = 8 megabit
									//65536); // 65536 byte = 512 kilobit (default)
							}

							// DEBUG
							// if (data.isLastElement)
							// 	printLocalResult((size_t*)&localResult, out);

							#pragma unroll
							for (ushort bitOffset = 0; bitOffset < CHUNK_BITS; ++bitOffset)
							{
								const Counter counter =
									// Avoid additional port for init
									(!countersInitialized? 0 : counters[chunkIndex][bitOffset])
									+ bitvector[bitOffset];

								counters[chunkIndex][bitOffset] = counter;

								// TODO: only write localResult when threshold is initialised (if (data.isLastElement))?
								localResult[bitOffset] = counter >= threshold;

								// DEBUG
								// if (data.isLastElement)
								// {
								// 	// out << "bitOffset: counter threshold" << sycl::endl;
								// 	out << bitOffset << ": " << counter << " " << threshold << sycl::endl;
								// 	printLocalResult((uint64_t*)&localResult, out);
								// }
							}

							if (data.isLastElement)
							{
								// DEBUG
								// out << "Result at " << static_cast<size_t>(queryIndex * CHUNKS + chunkIndex) << ":";
								// printLocalResult((uint64_t*)&localResult, out);

								CollectorPipes::PipeAt<id>::write(localResult);
							}
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});