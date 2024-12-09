		handler.single_task<Distributor>([=]() [[intel::kernel_args_restrict]]
		{
			sycl::ext::intel::host_ptr<const char> queries_ptr_casted(queries_ptr);
			sycl::ext::intel::host_ptr<const HostSizeType> querySizes_ptr_casted(querySizes_ptr);

			size_t queries_ptr_index = 0;

			for (size_t i = 0; i < numberOfQueries; i++)
			{
				DistributorToMinimizerData data;
				data.size = PrefetchingLSU::load(querySizes_ptr_casted + i);

				for (size_t j = 0; j < data.size; j++)
				{
					data.query[j] = PrefetchingLSU::load(queries_ptr_casted + queries_ptr_index);
					queries_ptr_index++;
				}

				switch (i % KERNEL_COPYS)
				{
				case 0:
					DistributorPipes::PipeAt<0>::write(data);
					break;
#if KERNEL_COPYS > 1
				case 1:
					DistributorPipes::PipeAt<1>::write(data);
					break;
#endif
#if KERNEL_COPYS > 2
				case 2:
					DistributorPipes::PipeAt<2>::write(data);
					break;
#endif
#if KERNEL_COPYS > 3
				case 3:
					DistributorPipes::PipeAt<3>::write(data);
					break;
#endif
				}
			}
		});