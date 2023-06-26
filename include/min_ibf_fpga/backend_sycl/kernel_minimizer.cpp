		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor queries(queries_buffer, handler, sycl::read_only);
			sycl::accessor querySizes(querySizes_buffer, handler, sycl::read_only);

			sycl::stream out(65536, 256, handler); // DEBUG
			handler.single_task<MinimizerKernel>([=]() [[intel::kernel_args_restrict]]
			{
				using QueryIndex = types::QueryIndex;
				using Hash = types::Hash;
				using Minimizer = types::Minimizer;

				// Prefetching requires pointer
				auto querySizes_ptr = querySizes.get_pointer();
				auto queries_ptr = queries.get_pointer();

				QueryIndex queryOffset = queriesOffset;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					const QueryIndex querySize = PrefetchingLSU::load(querySizes_ptr + static_cast<size_t>(querySizesOffset) + static_cast<size_t>(queryIndex));

					const QueryIndex localQueryOffset = queryOffset;
					queryOffset += querySize;

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
							queryBuffer[constants::min_ibf_k - 1] = PrefetchingLSU::load(queries_ptr + static_cast<size_t>(localQueryOffset + iteration));

						// Shift register: hash buffer
						#pragma unroll
						for (ushort i = 0; i < constants::number_of_kmers_per_window - 1; ++i)
							hashBuffer[i] = hashBuffer[i + 1];

						hashBuffer[constants::number_of_kmers_per_window - 1] = minimizer_kernel_t::extractHash(queryBuffer); // , out); // DEBUG

						const Minimizer minimizer = minimizer_kernel_t::findMinimizer(hashBuffer);
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

							MinimizerToIBFPipes::PipeAt<id>::write(data);
						}
					}
				}
			});
		});