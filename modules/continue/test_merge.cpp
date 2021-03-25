/*
 * test_merge.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: Joost Huizinga
 */
#ifndef STATIC_BOOST
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE continue_merge
#include <boost/test/unit_test.hpp>
#include <modules/continue/global_options.hpp>
#include "stat_merge_one_line_gen_files.hpp"

/**
* Make available program's arguments to all tests, receiving
* this fixture.
*/
struct ArgsFixture {
   ArgsFixture(): argc(boost::unit_test::framework::master_test_suite().argc),
           argv(boost::unit_test::framework::master_test_suite().argv){}
   int argc;
   char **argv;
};

class ChangeDir {
    boost::filesystem::path _original_path;
public:
    ChangeDir(boost::filesystem::path new_dir){
        std::cout << "Moving into directory: " << new_dir << std::endl;
        _original_path = boost::filesystem::current_path();
        boost::filesystem::current_path(new_dir);
    }

    ~ChangeDir(){
        toPreviousDir();
    }

    void toPreviousDir(){
        std::cout << "Returning to directory: " << _original_path << std::endl;
        boost::filesystem::current_path(_original_path);
    }
};

BOOST_FIXTURE_TEST_CASE(continue_merge_test, ArgsFixture)
{
    sferes::options::parse_and_init(argc, argv, false);

    // Setup our test directory
    boost::filesystem::path root("test_merge");
    BOOST_REQUIRE(boost::filesystem::create_directory(root));
    ChangeDir scoped_chang_of_directory(root);
    boost::filesystem::path dir1("test_merge_1");
    BOOST_REQUIRE(boost::filesystem::create_directory(dir1));
    boost::filesystem::path dir2("test_merge_2");
    BOOST_REQUIRE(boost::filesystem::create_directory(dir2));

    // Merge two non-overlapping files
    {
        // Construct the ground truth
        std::vector<std::string> groundTruth;
        groundTruth.push_back("#gen value");
        groundTruth.push_back("0 0.1");
        groundTruth.push_back("100 0.2");
        groundTruth.push_back("200 0.3");
        groundTruth.push_back("300 0.4");

        // Write the first partial file
        boost::filesystem::ofstream output1(dir1/"merge_test.dat");
        output1 << groundTruth[0] << "\n";
        output1 << groundTruth[1] << "\n";
        output1 << groundTruth[2] << "\n";
        output1.close();

        // Write the second partial file
        boost::filesystem::ofstream output2(dir2/"merge_test.dat");
        output2 << groundTruth[0] << "\n";
        output2 << groundTruth[3] << "\n";
        output2 << groundTruth[4] << "\n";
        output2.close();

        // Merge the files
        sferes::stat::mergeOneLineGenFiles(".", dir2.string(), "merge_test.dat", sferes::stat::oneLineMultiGen);

        // Check the result
        std::string line;
        std::vector<std::string> fileWritten;
        boost::filesystem::ifstream result(dir2/"merge_test.dat");
        while(std::getline(result, line)){
            fileWritten.push_back(line);
        }
        result.close();
        BOOST_CHECK_EQUAL_COLLECTIONS(fileWritten.begin(), fileWritten.end(), groundTruth.begin(), groundTruth.end());

        // Remove the result
        boost::filesystem::remove(dir2/"merge_test.dat");
    }

    // Merge two overlapping files
    {
        // Construct the ground truth
        std::vector<std::string> groundTruth;
        groundTruth.push_back("#gen value");
        groundTruth.push_back("0 0.1");
        groundTruth.push_back("100 0.2");
        groundTruth.push_back("200 0.3");
        groundTruth.push_back("300 0.4");

        // Write the first partial file
        boost::filesystem::ofstream output1(dir1/"merge_test.dat");
        output1 << groundTruth[0] << "\n";
        output1 << groundTruth[1] << "\n";
        output1 << groundTruth[2] << "\n";
        output1 << groundTruth[3] << "\n";
        output1.close();

        // Write the second partial file
        boost::filesystem::ofstream output2(dir2/"merge_test.dat");
        output2 << groundTruth[0] << "\n";
        output2 << groundTruth[3] << "\n";
        output2 << groundTruth[4] << "\n";
        output2.close();

        // Merge the files
        sferes::stat::mergeOneLineGenFiles(".", dir2.string(), "merge_test.dat", sferes::stat::oneLineMultiGen);

        // Check the result
        std::string line;
        std::vector<std::string> fileWritten;
        boost::filesystem::ifstream result(dir2/"merge_test.dat");
        while(std::getline(result, line)){
            fileWritten.push_back(line);
        }
        result.close();
        BOOST_CHECK_EQUAL_COLLECTIONS(fileWritten.begin(), fileWritten.end(), groundTruth.begin(), groundTruth.end());

        // Remove the result
        boost::filesystem::remove(dir2/"merge_test.dat");
    }

    // Merge two overlapping one-line-per-gen files
    {
        // Construct the ground truth
        std::vector<std::string> groundTruth;
        groundTruth.push_back("#gen value");
        groundTruth.push_back("0 0.1");
        groundTruth.push_back("1 0.2");
        groundTruth.push_back("2 0.3");
        groundTruth.push_back("3 0.4");

        // Write the first partial file
        boost::filesystem::ofstream output1(dir1/"merge_test.dat");
        output1 << groundTruth[0] << "\n";
        output1 << groundTruth[1] << "\n";
        output1 << groundTruth[2] << "\n";
        output1 << groundTruth[3] << "\n";
        output1.close();

        // Write the second partial file
        boost::filesystem::ofstream output2(dir2/"merge_test.dat");
        output2 << groundTruth[0] << "\n";
        output2 << groundTruth[3] << "\n";
        output2 << groundTruth[4] << "\n";
        output2.close();

        // Merge the files
        sferes::stat::mergeOneLineGenFiles(".", dir2.string(), "merge_test.dat", sferes::stat::oneLineGen);

        // Check the result
        std::string line;
        std::vector<std::string> fileWritten;
        boost::filesystem::ifstream result(dir2/"merge_test.dat");
        while(std::getline(result, line)){
            fileWritten.push_back(line);
        }
        result.close();
        BOOST_CHECK_EQUAL_COLLECTIONS(fileWritten.begin(), fileWritten.end(), groundTruth.begin(), groundTruth.end());

        // Remove the result
        boost::filesystem::remove(dir2/"merge_test.dat");
    }

    // Merge two non-overlapping files containing only a single line per file
    {
        // Construct the ground truth
        std::vector<std::string> groundTruth;
        groundTruth.push_back("#gen value");
        groundTruth.push_back("0 0.1");
        groundTruth.push_back("2 0.3");

        // Write the first partial file
        boost::filesystem::ofstream output1(dir1/"merge_test.dat");
        output1 << groundTruth[0] << "\n";
        output1 << groundTruth[1] << "\n";
        output1.close();

        // Write the second partial file
        boost::filesystem::ofstream output2(dir2/"merge_test.dat");
        output2 << groundTruth[0] << "\n";
        output2 << groundTruth[2] << "\n";
        output2.close();

        // Merge the files
        sferes::stat::mergeOneLineGenFiles(".", dir2.string(), "merge_test.dat", sferes::stat::oneLineMultiGen);

        // Check the result
        std::string line;
        std::vector<std::string> fileWritten;
        boost::filesystem::ifstream result(dir2/"merge_test.dat");
        while(std::getline(result, line)){
            fileWritten.push_back(line);
        }
        result.close();
        BOOST_CHECK_EQUAL_COLLECTIONS(fileWritten.begin(), fileWritten.end(), groundTruth.begin(), groundTruth.end());

        // Remove the result
        boost::filesystem::remove(dir2/"merge_test.dat");
    }

    scoped_chang_of_directory.toPreviousDir();
    boost::filesystem::remove_all(root);
}



