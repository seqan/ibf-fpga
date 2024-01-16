			sycl::accessor ibfData(ibfData_buffer, handler, sycl::read_only);
			sycl::accessor thresholds(thresholds_buffer, handler, sycl::read_only);
			sycl::accessor result(result_buffer, handler, sycl::write_only);

			handler.single_task<IbfKernel>([=]() [[intel::kernel_args_restrict]]
			{
				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
					#define II (UNSAFELEN - CHUNKS)

					#if II > 1
						Counter counters[CHUNKS][CHUNK_BITS]; // __attribute__((register))
					#else
						Counter __attribute__((
								memory,
								numbanks(1),
								bankwidth(CHUNK_BITS * sizeof(Counter)),
								singlepump,//singlepump,
								max_replicates(1)))
							counters[CHUNKS][CHUNK_BITS];
					#endif
					bool countersInitialized = 0;

					QueryIndex numberOfHashes = 1;

					MinimizerToIBFData data;

					#if II <= 1
						#pragma ivdep array(counters)
					#endif
					do
					{
						data = MinimizerToIBFPipes::PipeAt<id>::read();

						const QueryIndex localNumberOfHashes = numberOfHashes++;

						Counter threshold;

						if (data.isLastElement) {
								//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
								// manual inline of getThreshold() because of thresholds accessor
								const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

								HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
								index = index < maximalIndex? index : maximalIndex;

								threshold = (thresholds[static_cast<size_t>(index)] + 2).to_uint();
						}

						HostSizeType binOffsets[HASH_COUNT];

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
							binOffsets[seedIndex] = calculateBinIndex(data.hash,
								seedIndex, hashShift, binSize) * CHUNKS_PER_BIN;

						for (unsigned char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
						{
							Chunk bitvector = ~(Chunk)0;
							Chunk localResult = 0;

							// Unroll: Burst-coalesced over chunks per seed
							#pragma unroll
							for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
								bitvector &= //__burst_coalesced_cached_load(
									/*&*/ibfData[static_cast<size_t>(binOffsets[seedIndex]) + chunkIndex];//,
									//1048576); // 1 MiB = 8 megabit
									//65536); // 65536 byte = 512 kilobit (default)

							#pragma unroll
							for (ushort bitOffset = 0; bitOffset < CHUNK_BITS; ++bitOffset)
							{
								const Counter counter =
									// Avoid additional port for init
									(!countersInitialized? 0 : counters[chunkIndex][bitOffset])
									+ bitvector[bitOffset];

								counters[chunkIndex][bitOffset] = counter;

								localResult[bitOffset] = counter >= threshold;

								/*if (data.isLastElement)
									printf("%d %d\n", counter, threshold);*/
							}

							if (data.isLastElement)
								result[static_cast<size_t>(queryIndex * CHUNKS + chunkIndex)] = localResult;
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});