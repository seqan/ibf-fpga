			handler.single_task<MinimizerKernel<id>>([=]() [[intel::kernel_args_restrict]]
			{
				for (QueryIndex queryIndex = 0; queryIndex < localNumberOfQueries; queryIndex++)
				{
					DistributorToMinimizerData query;
					query = DistributorPipes::PipeAt<id>::read();

					const QueryIndex iterations = query.size;

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
						if (iteration < query.size)
							queryBuffer[MIN_IBF_K - 1] = query.query[iteration];

						// Shift register: hash buffer
						#pragma unroll
						for (ushort i = 0; i < NUMBER_OF_KMERS_PER_WINDOW - 1; ++i)
							hashBuffer[i] = hashBuffer[i + 1];

						hashBuffer[NUMBER_OF_KMERS_PER_WINDOW - 1] = extractHash(queryBuffer); // , out); // DEBUG

						const Minimizer minimizer = findMinimizer(hashBuffer);

						// After WINDOW_SIZE many iterations, we have to write the first minimizer.
						// Initialise lastMinimizer here such that skipMinimizer will be false for the next iteration.
						if (iteration == WINDOW_SIZE - 1)
						{
							lastMinimizer = minimizer;
						}

						// If false, a new minimizer was found
						const bool skipMinimizer = lastMinimizer.position != 0 && lastMinimizer.hash == minimizer.hash;
						// If true, we are at the last element
						const bool lastElement = iteration > iterations - 1;
						
						if (iteration >= WINDOW_SIZE && (!skipMinimizer || lastElement))
						{
							MinimizerToIBFData data;
							data.isLastElement = lastElement;
							data.hash = lastMinimizer.hash;

							MinimizerToIBFPipes::PipeAt<id>::write(data);
							lastMinimizer = minimizer;
						}
						if (lastMinimizer.position != 0)
							--lastMinimizer.position;
					}
				}
			});