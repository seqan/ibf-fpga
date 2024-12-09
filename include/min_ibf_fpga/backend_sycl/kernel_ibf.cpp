			handler.single_task<IbfKernel<id>>([=]() [[intel::kernel_args_restrict]]
			{
				sycl::ext::intel::device_ptr<const HostSizeType> thresholds_ptr_casted(thresholds_ptr);
				sycl::ext::intel::device_ptr<const Chunk> ibfData_ptr_casted(ibfData_ptr);

				HostSizeType thresholds[THRESHOLDS_CACHE_SIZE];

				const HostSizeType thresholdsMaximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

				for (ushort i = 0; i <= thresholdsMaximalIndex; i++)
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
						}

						HostSizeType binOffsets[HASH_COUNT];

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
						{
							binOffsets[seedIndex] = calculateBinIndex(data.hash, seedIndex, hashShift, binSize) * CHUNKS_PER_BIN;
						}

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

							#pragma unroll
							for (ushort bitOffset = 0; bitOffset < CHUNK_BITS; ++bitOffset)
							{
								const Counter counter =
									// Avoid additional port for init
									(!countersInitialized? 0 : counters[chunkIndex][bitOffset])
									+ bitvector[bitOffset];

								counters[chunkIndex][bitOffset] = counter;

								localResult[bitOffset] = counter >= threshold;
							}

							if (data.isLastElement)
							{
								CollectorPipes::PipeAt<id>::write(localResult);
							}
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});