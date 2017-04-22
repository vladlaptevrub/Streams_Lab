#include <iostream>
#include <vector>
#include "gtest/gtest.h"

#include "Stream.h"

TEST (mapTest, lambdaTest)
{
	auto test_stream = MakeStream({2, 4});
	auto new_test_stream = test_stream | map([](int x){return x*x;});

    std::vector<int> vec = std::move(new_test_stream.stream);

    for (auto i = new_test_stream.operations.begin(); i < new_test_stream.operations.end(); ++i){
        vec = std::move((*i)->apply_to(std::move(vec)));
    }

	std::vector<int> test_vector({4, 16});
	EXPECT_EQ(test_vector, vec);
}

TEST (reduceTest, lambdaTest)
{
	auto test_stream = MakeStream({2, 4});
	int new_val = test_stream | reduce([](int x, int y){return x*y;});
	int test_val = 8;
	EXPECT_EQ(test_val, new_val);
}

TEST (sumTest, expectResult)
{
	auto test_stream = MakeStream({2, 4});
	int new_val = test_stream | sum();
	int test_val = 6;
	EXPECT_EQ(test_val, new_val);
}

TEST (skipTest, lowSkip)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
    auto new_test_stream = test_stream | skip(2);

    std::vector<int> vec = std::move(new_test_stream.stream);

    for (auto i = new_test_stream.operations.begin(); i < new_test_stream.operations.end(); ++i){
        vec = std::move((*i)->apply_to(std::move(vec)));
    }

	std::vector<int> test_vector({3, 4});
	EXPECT_EQ(test_vector, vec);
}

TEST (skipTest, biggerSkip)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
    auto new_test_stream = test_stream | skip(10);

    std::vector<int> vec = std::move(new_test_stream.stream);

    for (auto i = new_test_stream.operations.begin(); i < new_test_stream.operations.end(); ++i){
        vec = std::move((*i)->apply_to(std::move(vec)));
    }

	std::vector<int> test_vector;
	EXPECT_EQ(test_vector, vec);
}

TEST (filterTest, lambdaTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	auto new_test_stream = test_stream | filter([](int x){return x < 3;});

    std::vector<int> vec = std::move(new_test_stream.stream);

    for (auto i = new_test_stream.operations.begin(); i < new_test_stream.operations.end(); ++i){
        vec = std::move((*i)->apply_to(std::move(vec)));
    }

	std::vector<int> test_vector({1, 2});
	EXPECT_EQ(test_vector, vec);
}

TEST (filterTest, emptyTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	auto new_test_stream = test_stream | filter([](int x){return x < 0;});

    std::vector<int> vec = std::move(new_test_stream.stream);

    for (auto i = new_test_stream.operations.begin(); i < new_test_stream.operations.end(); ++i){
        vec = std::move((*i)->apply_to(std::move(vec)));
    }

	std::vector<int> test_vector;
	EXPECT_EQ(test_vector, vec);
}

TEST (nthTest, currentTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	int new_val = test_stream | nth(2);
	int test_val = 2;
	EXPECT_EQ(test_val, new_val);
}

TEST (nthTest, emptyTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	int new_val = test_stream | nth(6);
	int test_val = 0;
	EXPECT_EQ(test_val, new_val);
}

TEST (toVectorTest, equilTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	std::vector<int> new_vector = test_stream | to_vector();
	std::vector<int> test_vector({1, 2, 3, 4});
	EXPECT_EQ(test_vector, new_vector);
}

TEST (groupTest, equilTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});
	auto new_test_stream = test_stream | group(2);
	std::vector<int> test_vector_one({1, 2});
	std::vector<int> test_vector_two({3, 4});
	auto iter = new_test_stream.stream.begin();
	EXPECT_EQ(test_vector_one, *iter);
	iter++;
	EXPECT_EQ(test_vector_two, *iter);
}

TEST (groupTest, differentTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4, 5});
	auto new_test_stream = test_stream | group(3);
	std::vector<int> test_vector_one({1, 2, 3});
	std::vector<int> test_vector_two({4, 5});
	auto iter = new_test_stream.stream.begin();
	EXPECT_EQ(test_vector_one, *iter);
	iter++;
	EXPECT_EQ(test_vector_two, *iter);
}

TEST (groupTest, fullTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4, 5});
	auto new_test_stream = test_stream | group(5);
	std::vector<int> test_vector_one({1, 2, 3, 4, 5});
	auto iter = new_test_stream.stream.begin();
	EXPECT_EQ(test_vector_one, *iter);
}

TEST (pipeTest, multiPipeTest)
{
	auto test_stream = MakeStream({1, 2, 3, 4});

	int new_val = test_stream | map([](int x){return x*x;}) | skip(1) | reduce([](int x, int y){return x + y;});

	int test_val = 29;

	EXPECT_EQ(test_val, new_val);
}