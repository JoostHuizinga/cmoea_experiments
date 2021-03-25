/*
 * io_convenience.hpp
 *
 *  Created on: Oct 16, 2014
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_IO_CONVENIENCE_HPP_
#define EXP_MODULARITY_IO_CONVENIENCE_HPP_

#include <iostream>
#include <iterator>
#include <set>
#include <algorithm>    // std::sort
#include <iomanip>      // hex and dec

#include <boost/foreach.hpp>
// Fix for a bug in boost 1.55
// See: https://codeyarns.com/2017/09/20/undefined-reference-to-boost-copy_file/
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <sferes/dbg/dbg.hpp>




namespace sferes{

namespace io{

/**
 * Convenience operator for printing sets.
 */
template<typename ValueType>
std::ostream& operator<< (std::ostream &out, std::set<ValueType> set)
{
	std::copy( set.begin ()
			, set.end ()
			, std::ostream_iterator<ValueType> (out, "\n")
	);
	return out;
}


/**
 * Convenience operator for printing vectors.
 */
template<typename ValueType>
std::ostream& operator<< (std::ostream &out, std::vector<ValueType> vector)
{
	std::copy( vector.begin ()
			, vector.end ()
			, std::ostream_iterator<ValueType> (out, "\n")
	);
	return out;
}



/**
 * Function that returns true if the supplied filename is identical to the name defined in the constructor.
 */
class has_name_f{
public:
	has_name_f(std::string filename): _filename(filename)
{
		//nix
}

	bool operator()(const std::string& filename) const{
		return filename == _filename;

	}

private:
	std::string _filename;
};

/**
 * Returns the contents of the directory indicated by the string as a set of paths.
 */
std::set<boost::filesystem::path> getDirContents (const std::string& dirName){
	std::set<boost::filesystem::path> paths;
	std::copy
	( boost::filesystem::directory_iterator (dirName)
	, boost::filesystem::directory_iterator ()
	, std::inserter (paths, paths.end ())
	);

	return paths;
}


/**
 * Returns true if the supplied path points to an empty directory.
 * Returns false otherwise.
 */
bool isEmptyDirectory(const boost::filesystem::path& path){
	if(!boost::filesystem::is_directory(path)){
		return false;
	}
	std::set<boost::filesystem::path> contents = getDirContents(path.string());
	return contents.empty();
}



/**
 * Finds any files in the directory with dirName that matches the criteria indicated by the isTarget operator.
 * The file should be anywhere between min_depth and max_depth in the directory tree.
 *
 * Note, the comparator operator was initially going to be a boost regular expression,
 * but this required all experiments to be linked against the boost regular expression library.
 * This did not go well with the 'easy to add' goal of these files.
 */
template<typename IsTargetF>
std::set<boost::filesystem::path> find(
		const std::string& dirName,
		IsTargetF& isTarget,
		const size_t& min_depth,
		const size_t& max_depth,
		size_t depth = 0){
	dbg::trace trace("io", DBG_HERE);
	dbg::out(dbg::info, "io") << "Starting search of: " << dirName << std::endl;
	std::set<boost::filesystem::path> directoryContents = getDirContents(dirName);
	std::set<boost::filesystem::path> results;

	//If we reached the minimum depth, is the file we are searching for in this directory?
	if(depth >= min_depth){
		dbg::out(dbg::info, "io") << "Minimum search depth reached: " <<  min_depth << std::endl;
		BOOST_FOREACH( boost::filesystem::path path, directoryContents ){
			dbg::out(dbg::info, "io") << "Inspecting  path: " << path << std::endl;
			if(boost::filesystem::is_regular_file(path)){
				std::string fileNameFound = path.filename().string();
				if(isTarget(fileNameFound)){
					results.insert(path);
				}
			}
		}
	}

	//If we have not reached the maximum depth yet, is the file we are searching for in one of our sub-directories?
	if(depth < max_depth){
		dbg::out(dbg::info, "io") << "Maximum search depth not yet reached: " <<  max_depth << std::endl;
		BOOST_FOREACH( boost::filesystem::path path, directoryContents ){
			dbg::out(dbg::info, "io") << "Inspecting  path: " << path << std::endl;
			if(boost::filesystem::is_directory(path)){
				std::set<boost::filesystem::path> intermediate_results = find(path.string(), isTarget, min_depth, max_depth, depth+1);
				results.insert(intermediate_results.begin(), intermediate_results.end());
			}
		}
	}

	//The search failed, return an empty path.
	return results;
}


/**
 * Get the line 'line_nr' from the back of the file.
 */
std::string getLineFromBack(const boost::filesystem::path& filename, const size_t& line_nr = 1){
	dbg::trace trace("io", DBG_HERE);

	std::string result = "";

	boost::filesystem::ifstream fin(filename);
	char ch = ' ';
	size_t end_of_line_count = 0;

	if(fin.is_open()) {

		//Assume the file is terminated with a '\n'
		fin.seekg(0,std::ios_base::end);
		while(end_of_line_count < line_nr){
			fin.seekg(-2,std::ios_base::cur);
			if(fin.tellg() == 0){
				fin.seekg(0);
				std::cerr << "Warning: begin-of-file found before requested line was found." << std::endl;
				break;
			} else if(fin.tellg() == -1){
				fin.seekg(0);
				std::cerr << "warning: read error in file." << std::endl;
				break;
			}

			fin.get(ch);
			dbg::out(dbg::info, "linefromback") << std::hex << int(ch) << std::dec << " | " << ch << std::endl;
			if(ch == '\n'){
				dbg::out(dbg::info, "linefromback") << std::hex << int(ch) << " equals end-of-lin: " << int('\n') << std::dec << std::endl;
				++end_of_line_count;
			}
		}

		std::getline(fin,result);
		fin.close();
	} else {
		std::cerr << "Warning: file could not be opened." << std::endl;
	}

	return result;
}

/**
 * Deletes all empty folders in the indicated directory.
 */
void deleteEmptyDirectories(std::string directory = "."){
	dbg::trace trace("io", DBG_HERE);
	std::set<boost::filesystem::path> dir_contents = getDirContents(directory);
	BOOST_FOREACH(boost::filesystem::path path, dir_contents){
		if(isEmptyDirectory(path)){
			boost::filesystem::remove_all(path);
		}
	}
}



}
}



#endif /* EXP_MODULARITY_IO_CONVENIENCE_HPP_ */
