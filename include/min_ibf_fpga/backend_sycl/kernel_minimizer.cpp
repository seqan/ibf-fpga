			//sycl::stream out(65536, 256, handler); // DEBUG
			handler.single_task<MinimizerKernel>([=]() [[intel::kernel_args_restrict]]
			{
				sycl::host_ptr<const char> queries_ptr_casted(queries_ptr);
				sycl::host_ptr<const HostSizeType> querySizes_ptr_casted(querySizes_ptr);

				QueryIndex queryOffset = queriesOffset;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					const QueryIndex querySize = PrefetchingLSU::load(querySizes_ptr_casted + static_cast<size_t>(querySizesOffset) + static_cast<size_t>(queryIndex));

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
							queryBuffer[MIN_IBF_K - 1] = PrefetchingLSU::load(queries_ptr_casted + static_cast<size_t>(localQueryOffset + iteration));

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