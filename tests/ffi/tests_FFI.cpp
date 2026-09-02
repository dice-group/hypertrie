#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/ffi/hypertrie.h>

#include <iostream>
#include <string>
#include <random>

TEST_SUITE("ffi") {
	TEST_CASE("sanity check") {
		hypertrie *hyp = hypertrie_new_in_memory(HYPERTRIE_BOOL, 3);

		hypertrie_key key;
		key.size = 3;
		key.key[0] = 1;
		key.key[1] = 2;
		key.key[2] = 3;

		hypertrie_value value;
		value.discriminant = HYPERTRIE_BOOL;
		value.bool_ = true;
		hypertrie_set(hyp, &key, value);

		{
			hypertrie_value null_value;
			null_value.discriminant = HYPERTRIE_BOOL;
			null_value.bool_ = false;

			hypertrie *copy = hypertrie_clone(hyp);
			hypertrie_set(copy, &key, null_value);

			CHECK(hypertrie_get(copy, &key).bool_ == false);
			CHECK(hypertrie_get(hyp, &key).bool_ == true);
			hypertrie_destroy(copy);
		}

		hypertrie_value new_value = hypertrie_get(hyp, &key);
		CHECK(new_value.bool_);

		{
			hypertrie *op = hypertrie_new_in_memory(HYPERTRIE_INT64, 1);

			hypertrie_value opv;
			opv.discriminant = HYPERTRIE_INT64;
			opv.int64_ = 20;

			hypertrie_key opk1;
			opk1.size = 1;
			opk1.key[0] = 1;

			hypertrie_set(op, &opk1, opv);

			hypertrie_key opk2;
			opk2.size = 1;
			opk2.key[0] = 2;

			hypertrie_set(op, &opk2, opv);

			hypertrie const *operands[2] = {op, op};

			{
				hypertrie_einsum_solution_generator gen;
				hypertrie_result const res = hypertrie_einsum(operands, 2, "a,b->ab", HYPERTRIE_NO_TIMEOUT, HYPERTRIE_INT64, &gen);
				if (res == HYPERTRIE_FAILURE) {
					FAIL(std::string{strerror(errno)});
				}

				hypertrie *e = hypertrie_new_in_memory(HYPERTRIE_INT64, hypertrie_einsum_solution_generator_result_depth(&gen));
				hypertrie_consume_einsum_solution_generator(&gen, e);

				hypertrie_iterator iter;
				hypertrie_iterate(e, &iter);

				hypertrie_key_value kv;
				while (hypertrie_iterator_next(&iter, &kv) == HYPERTRIE_I_YIELDED) {
					for (size_t ix = 0; ix < kv.key.size; ++ix) {
						std::cout << kv.key.key[ix] << ' ';
					}

					std::cout << "-> " << kv.value.int64_ << std::endl;
				}

				hypertrie_iterator_destroy(&iter);
			}

			{
				hypertrie_einsum_solution_generator gen;
				hypertrie_result const res = hypertrie_einsum(operands, 2, "a,b->ab", HYPERTRIE_NO_TIMEOUT, HYPERTRIE_INT64, &gen);
				if (res == HYPERTRIE_FAILURE) {
					FAIL(std::string{strerror(errno)});
				}

				size_t const result_depth = hypertrie_einsum_solution_generator_result_depth(&gen);

				hypertrie_einsum_solution sol;
				sol.key.size = result_depth;
				sol.key.key = static_cast<hypertrie_key_part *>(malloc(result_depth * sizeof(hypertrie_key_part)));

				while (hypertrie_einsum_solution_generator_next(&gen, &sol) == HYPERTRIE_I_YIELDED) {
					for (size_t ix = 0; ix < sol.key.size; ++ix) {
						std::cout << sol.key.key[ix] << ' ';
					}

					std::cout << "-> " << sol.value.int64_ << std::endl;
				}

				hypertrie_einsum_solution_generator_destroy(&gen);
			}

			hypertrie_destroy(op);
		}

		{
			hypertrie_slice_key slice_key;
			slice_key.size = 3;
			slice_key.key[0] = 1;
			slice_key.mask = 0b001;

			hypertrie const *slice = hypertrie_slice(hyp, &slice_key);
			hypertrie *slice_c = hypertrie_new_in_memory(HYPERTRIE_FLOAT, hypertrie_depth(slice));
			hypertrie_copy_into(slice, slice_c);

			hypertrie_iterator iter;
			hypertrie_iterate(slice_c, &iter);

			hypertrie_key_value kv;
			while (hypertrie_iterator_next(&iter, &kv) == HYPERTRIE_I_YIELDED) {
				for (size_t ix = 0; ix < kv.key.size; ++ix) {
					std::cout << kv.key.key[ix] << ' ';
				}

				std::cout << "-> " << kv.value.float_ << std::endl;
			}

			hypertrie_iterator_destroy(&iter);
			hypertrie_destroy(slice_c);
			hypertrie_destroy(slice);
		}

		hypertrie_destroy(hyp);
	}

	TEST_CASE("metall") {
		std::string const metall_path = "/tmp/hypertrie_tests_ffi_" + std::to_string(std::random_device{}());
		char const *hypertrie_name = "test_hypertrie";

		hypertrie_value v;
		v.discriminant = HYPERTRIE_INT64;
		v.int64_ = 20;

		hypertrie_key k;
		k.size = 3;
		k.key[0] = 1;
		k.key[1] = 2;
		k.key[2] = 3;

		{
			metall_manager *manager = metall_create(metall_path.c_str());
			if (manager == nullptr) {
				FAIL("Unable to create datastore: ", strerror(errno));
			}

			hypertrie *hyp = hypertrie_create_persistent(HYPERTRIE_INT64, 3, manager, hypertrie_name);

			hypertrie_set(hyp, &k, v);

			hypertrie_destroy(hyp);
			metall_close(manager);
		}

		{
			metall_manager *manager = metall_open(metall_path.c_str());
			if (manager == nullptr) {
				FAIL("Unable to open datastore: ", strerror(errno));
			}

			hypertrie *hyp = hypertrie_open_persistent(manager, hypertrie_name);

			hypertrie_value v2 = hypertrie_get(hyp, &k);
			CHECK(v2.discriminant == v.discriminant);
			CHECK(v2.int64_ == v.int64_);

			hypertrie_destroy(hyp);
			CHECK(hypertrie_destroy_persistent(manager, hypertrie_name));

			metall_close(manager);
			metall_remove(metall_path.c_str());
		}
	}
}
