			handler.single_task<MinimizerKernel<id>>([=]() [[intel::kernel_args_restrict]]
			{
				sycl::ext::intel::host_ptr<const char> queries_ptr_casted(queries_ptr);

				QueryIndex queryOffset = id * queriesPerKernel * queryLength;

				for (QueryIndex queryIndex = 0; queryIndex < queriesPerKernel; queryIndex++)
				{
					const QueryIndex iterations =
						INITIALIZATION_ITERATIONS // Fill query and hash buffer initially
						+ queryLength - WINDOW_SIZE + 1;

					char queryBuffer[MIN_IBF_K] = {0};
					Hash hashBuffer[NUMBER_OF_KMERS_PER_WINDOW] = {0};

					// Set initial element's position to 0, so the first real element will never be skipped
					Minimizer lastMinimizer = {0, 0};

					for (QueryIndex iteration = 0; iteration <= iterations; iteration++)
					{
						// Shift register: Query buffer
						#pragma unroll
						for (unsigned char i = 0; i < MIN_IBF_K - 1; ++i)
							queryBuffer[i] = queryBuffer[i + 1];

						// Query as long as elements are left, then only do calculations (end phase)
						if (iteration < queryLength)
							queryBuffer[MIN_IBF_K - 1] = PrefetchingLSU::load(queries_ptr_casted + static_cast<size_t>(queryOffset + iteration));

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

							MinimizerToIBFPipes::PipeAt<id>::write(data);
						}
					}
					queryOffset += queryLength;
				}
			});