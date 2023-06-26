		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor ibfData(ibfData_buffer, handler, sycl::read_only);
			sycl::accessor thresholds(thresholds_buffer, handler, sycl::read_only);
			sycl::accessor result(result_buffer, handler, sycl::write_only);

			handler.single_task<IbfKernel>([=]() [[intel::kernel_args_restrict]]
			{
				using QueryIndex = typename types::QueryIndex;
				using Counter = typename types::Counter;
				using Chunk = typename types::Chunk;
				using HostSizeType = typename types::HostSizeType;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
					#define II (UNSAFELEN - CHUNKS)

					#if II > 1
						Counter counters[constants::chunks][constants::chunk_bits]; // __attribute__((register))
					#else
						Counter __attribute__((
								memory,
								numbanks(1),
								bankwidth(constants::chunk_bits * sizeof(Counter)),
								singlepump,//singlepump,
								max_replicates(1)))
							counters[constants::chunks][constants::chunk_bits];
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

						HostSizeType binOffsets[constants::hash_count];

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < constants::hash_count; ++seedIndex)
							binOffsets[seedIndex] = ibf_kernel_t::calculateBinIndex(data.hash,
								seedIndex, hashShift, binSize) * constants::chunks_per_bin;

						for (unsigned char chunkIndex = 0; chunkIndex < constants::chunks; chunkIndex++)
						{
							Chunk bitvector = ~(Chunk)0;
							Chunk localResult = 0;

							// Unroll: Burst-coalesced over chunks per seed
							#pragma unroll
							for (unsigned char seedIndex = 0; seedIndex < constants::hash_count; ++seedIndex)
								bitvector &= //__burst_coalesced_cached_load(
									/*&*/ibfData[static_cast<size_t>(binOffsets[seedIndex]) + chunkIndex];//,
									//1048576); // 1 MiB = 8 megabit
									//65536); // 65536 byte = 512 kilobit (default)

							#pragma unroll
							for (ushort bitOffset = 0; bitOffset < constants::chunk_bits; ++bitOffset)
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
							{
								size_t result_idx = (size_t)queryIndex * constants::chunks + chunkIndex;
								result[result_idx] = localResult;
							}
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});
		});