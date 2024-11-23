		handler.single_task<Collector>([=]() [[intel::kernel_args_restrict]]
		{
			sycl::ext::intel::host_ptr<Chunk> result_ptr_casted(result_ptr);

			for (QueryIndex queryIndex = 0; queryIndex < static_cast<QueryIndex>(numberOfQueries); queryIndex++)
			{
				unsigned char pipeIndex = queryIndex % KERNEL_COPYS;

				for (unsigned char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
				{
					Chunk chunk;

					switch (pipeIndex)
					{
					case 0:
						chunk = CollectorPipes::PipeAt<0>::read();
						break;
#if KERNEL_COPYS > 1
					case 1:
						chunk = CollectorPipes::PipeAt<1>::read();
						break;
#endif
#if KERNEL_COPYS > 2
					case 2:
						chunk = CollectorPipes::PipeAt<2>::read();
						break;
#endif
#if KERNEL_COPYS > 3
					case 3:
						chunk = CollectorPipes::PipeAt<3>::read();
						break;
#endif
					}

					result_ptr_casted[static_cast<size_t>(queryIndex * CHUNKS + chunkIndex)] = chunk;
				}
			}
		});