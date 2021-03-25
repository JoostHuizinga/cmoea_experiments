/*
 * stat_merge_one_line_gen_files.hpp
 *
 *  Created on: Oct 16, 2014
 *      Author: Joost Huizinga
 *
 * File that allows merging two output files, provided that:
 * - The file contains one line per generation
 * - The first number on each line indicates the number of that generation
 * WARNING: DO NOT USE IT ON FILES FOR WHICH THESE ASSUMPTIONS DO NOT HOLD
 * THIS FUNCTION WILL DELETE, OVERWRITE, AND MANGLE YOUR DATA!
 */

#ifndef EXP_MODULARITY_STAT_MERGE_ONE_LINE_GEN_FILES_HPP_
#define EXP_MODULARITY_STAT_MERGE_ONE_LINE_GEN_FILES_HPP_

#include <sferes/dbg/dbg.hpp>
#include <boost/foreach.hpp>
#include <modules/datatools/io_convenience.hpp>

namespace sferes{

namespace stat{

enum LineGenType{
    oneLineGen,
    oneLineMultiGen
};

//Brings the << operator into scope for vectors and sets
using namespace sferes::io;

int getGen(std::string line){
    int result = -1;
    std::istringstream line_stream(line);
    if(!(line_stream >> result)){
        result = -1;
    }
    return result;
}

/**
 * This object represents the data associated with a single 'one line per generation' file.
 *
 * It stores:
 * - The first generation of the file.
 * - The last generation of the file.
 * - Whether or not this file contains a header.
 * - The length of the last line.
 * - The path to the file.
 */
class LineGen{
public:

    /**
     * Constructs a OneLineGen object from a file path.
     *
     * If the constructor successfully parses the file, the object contains
     * information about the first and last generations of the file.
     *
     * If the constructor fails to parse the file the first generation and last
     * generation will be set to -1.
     *
     * @param file The path to file.
     */
    LineGen(boost::filesystem::path file):
        _file(file), _first_gen(-1), _last_gen(-1), _has_header(false)
    {
        dbg::trace trace("continue", DBG_HERE);
        int gen;
        std::string line;
        boost::filesystem::ifstream filestream(_file);

        //Read the first generation
        if(std::getline(filestream, line)){
            dbg::out(dbg::info, "continue") << "First line: " << line << std::endl;
            _first_gen = getGen(line);
            if(_first_gen == -1){
                dbg::out(dbg::info, "continue") << "First line is header" << std::endl;
                _has_header = true;
            }
        }

        //If this file has a header, get the first generation from the second line
        if(_has_header){
            if(std::getline(filestream, line)){
                dbg::out(dbg::info, "continue") << "Second line: " << line << std::endl;
                _first_gen = getGen(line);
            }
        }

        _last_gen = _first_gen;

        //Read other lines, checking for generation consistency
        if(_first_gen != -1){
            initLastGen(filestream);
        } else{
            dbg::out(dbg::info, "continue") << "Generation not readable, file is bad." << std::endl;
        }

        filestream.close();
    }

    virtual ~LineGen(){
        // There is nothing that needs to be explicitly destroyed
    }

    virtual void initLastGen(boost::filesystem::ifstream& filestream){
        std::string line;
        int gen;
        while(std::getline(filestream, line)){
            gen = getGen(line);
            dbg::out(dbg::info, "continue") << "Line: " << line << std::endl;
            dbg::out(dbg::info, "continue") << "Gen: " << gen << " prev gen: " << _last_gen << std::endl;
            _last_gen = gen;
        }

    }

    /**
     * Returns the first generation contained in the file.
     *
     * Returns -1 if the file failed to be parsed.
     */
    int getFirstGen() const{
        return _first_gen;
    }

    /**
     * Returns the last generation contained in the file.
     *
     * Returns -1 if the file failed to be parsed.
     */
    int getLastGen() const{
        return _last_gen;
    }

    /**
     * Returns the length of the last line of the file.
     *
     * Returns 0 if the file failed to be parsed (but remember, 0 can also be a valid
     * value for line length, call good() to distinguish the two cases).
     */
    size_t getLengthLastLine() const{
        return io::getLineFromBack(_file, 1).size();
    }

    /**
     * Returns the path to the file.
     */
    boost::filesystem::path getPath(){
        return _file;
    }

    /**
     * Returns true of the file has a header, false otherwise.
     */
    bool hasHeader(){
        return _has_header;
    }

    /**
     * Returns true if the file was parsed successfully, false otherwise.
     */
    bool good(){
        return _first_gen > -1;
    }

protected:
    boost::filesystem::path _file;
    int _first_gen;
    int _last_gen;
    bool _has_header;
};


/**
 * This object represents the data associated with a single 'one line per generation' file.
 *
 * It stores:
 * - The first generation of the file.
 * - The last generation of the file.
 * - Whether or not this file contains a header.
 * - The length of the last line.
 * - The path to the file.
 */
class OneLineGen: public LineGen{
public:
    OneLineGen(boost::filesystem::path file): LineGen(file){
        //nix
    }

    virtual void initLastGen(boost::filesystem::ifstream& filestream){
        std::string line;
        int gen;
        while(std::getline(filestream, line)){
            gen = getGen(line);
            dbg::out(dbg::info, "continue") << "Line: " << line << std::endl;
            dbg::out(dbg::info, "continue") << "Gen: " << gen << " prev gen: " << _last_gen << std::endl;
            if(_last_gen != (gen-1)){
                dbg::out(dbg::info, "continue") << "This gen and prev gen are not consecutive, abort." << std::endl;
                break;
            }
            _last_gen = gen;
        }
    }

    virtual ~OneLineGen(){
        // There is nothing that needs to be explicitly destroyed
    }
};



/**
 * This object represents the data associated with a single 'one line for multiple generations' file.
 *
 * It stores:
 * - The first generation of the file.
 * - The last generation of the file.
 * - Whether or not this file contains a header.
 * - The length of the last line.
 * - The path to the file.
 */
class OneLineMultiGen: public LineGen{
public:
    OneLineMultiGen(boost::filesystem::path file): LineGen(file){
        //nix
    }

    virtual void initLastGen(boost::filesystem::ifstream& filestream){
        std::string line;
        int gen;
        while(std::getline(filestream, line)){
            gen = getGen(line);
            dbg::out(dbg::info, "continue") << "Line: " << line << std::endl;
            dbg::out(dbg::info, "continue") << "Gen: " << gen << " prev gen: " << _last_gen << std::endl;
            if(_last_gen >= gen){
                dbg::out(dbg::info, "continue") << "This gen and prev gen are not consecutive, abort." << std::endl;
                break;
            }
            _last_gen = gen;
        }
    }

    virtual ~OneLineMultiGen(){
        // There is nothing that needs to be explicitly destroyed
    }
};

/**
 * Compares OneLineGen files according to the generations they hold.
 *
 * gen1 will be considered less than gen2 if:
 * - The first generation of gen1 precedes the first generation of gen2
 * - If both files start at the same generation, gen1 is considered
 *   smaller than gen2 if its last generation is GREATER THAN the last generation of gen2.
 *   This means that, for two files starting at the same generation,
 *   the one with the most data will be considered the smallest,
 *   and it will thus appear as the first file in a sorted list.
 *   This is useful because we usually only care about the file that
 *   describes the most generations, other files were probably cut short.
 * - If both files have the same start and end generation, the file
 *   with the longest last line will be considered the smallest.
 *   This is useful because, if there is a difference in line length,
 *   the file with the shorter line probably was cut short while writing that line.
 *   Once again we make sure that we put the file with the most data first,
 *   such that we can ignore subsequent files.
 */
bool operator<(const LineGen& gen1, const LineGen& gen2){
	if(gen1.getFirstGen() != gen2.getFirstGen()){
		return gen1.getFirstGen() < gen2.getFirstGen();
	} else if(gen1.getLastGen() != gen2.getLastGen()){
		return gen1.getLastGen() > gen2.getLastGen();
	} else{
		return gen1.getLengthLastLine() > gen2.getLengthLastLine();
	}
}

/**
 * Merges all output files with filename into a single file with filename.
 *
 * Files to merge are found by doing a local search for all files with the supplied filename,
 * starting form the current directory. Make sure the current directory is correct, because
 * this function will delete all files with the supplied filename that it finds.
 *
 * Will only search in direct sub-directories of the current directory.
 *
 * @param resultDirectory The directory were the merged file should be placed.
 * @param filename        The name of the files that should be merged into a single file with this same name.
 */
void mergeOneLineGenFiles(const std::string& searchDirectory,
		const std::string& resultDirectory,
		const std::string& filename,
		LineGenType lineGenType = oneLineGen)
{
	dbg::trace trace("continue", DBG_HERE);

	//Find files to be merged
	io::has_name_f is_target_file(filename);
	std::set<boost::filesystem::path> files = io::find(searchDirectory, is_target_file, 1, 1);
	dbg::out(dbg::info, "continue") << "For '" << filename << "' the following files were found:\n" << files << std::endl;

    if(files.size() == 1){
        dbg::out(dbg::info, "continue") << "One file found, copying..." << std::endl;
        boost::filesystem::path origin = *files.begin();
        boost::filesystem::path target(resultDirectory + "/" + filename);
        try{
            boost::filesystem::copy_file(origin, target);
            boost::filesystem::remove(origin);
        } catch (const boost::filesystem::filesystem_error& e){
            std::cerr << "Error: " << e.what() << std::endl;
        }
        return;
    }

	if(files.size() <= 1){
		dbg::out(dbg::info, "continue") << "Not enough files to be merged (min: 2, found: "<< files.size() << " ), aborting." << std::endl;
		return;
	}

	//Sort the file according to the data they hold
	std::vector<LineGen> files_sorted;
	BOOST_FOREACH(boost::filesystem::path path, files){
	    if(lineGenType == oneLineGen){
	        files_sorted.push_back(OneLineGen(path));
	    } else if (lineGenType == oneLineMultiGen){
	        files_sorted.push_back(OneLineMultiGen(path));
	    } else{
	        std::cerr << "Error: unknown line-per-generation type." << std::endl;
	        return;
	    }
	}
	dbg::assertion(DBG_ASSERTION(files_sorted.size() > 1));
	std::sort(files_sorted.begin(), files_sorted.end());

	//Open a temporary output file if the temporary name if available
	boost::filesystem::ofstream output;
	std::string temp_log_file = resultDirectory + "/" + filename + "_temp";
	boost::filesystem::path temp_log_path(temp_log_file);
	if(boost::filesystem::exists(temp_log_path)){
		std::cerr << "Temp file name " << temp_log_file << " already in use! Aborting merge." << std::endl;
		return;
	}

	//Define some variables
	std::string line;
	LineGen* next_file;
	LineGen* current_file;
	size_t next_file_index = 0;

	//Skip bad files (they will be sorted to the front)
	while(next_file_index < files_sorted.size()){
		current_file = &files_sorted[next_file_index];
		if(current_file->good()) break;
		++next_file_index;
	}

	// Make sure that at least 1 good file was found
	if(next_file_index >= files_sorted.size()){
	    dbg::out(dbg::info, "continue") << "No good files found. Aborting merge." << std::endl;
	    return;
	}

	// Open our new output file
	output.open(temp_log_path);
	if(!output.good()){
        std::cerr << "Could not open: " << temp_log_path << " ! Aborting merge." << std::endl;
        return;
	}

	// Write the header if our file has one
	if(current_file->hasHeader()){
		boost::filesystem::ifstream current_file_stream(current_file->getPath());
		if(std::getline(current_file_stream, line)){
			dbg::out(dbg::info, "continue") << "Header line: " << line << std::endl;
			output << line << '\n';
		}
		current_file_stream.close();
	}

	// Write data
	while(next_file_index<files_sorted.size()){
		current_file = &files_sorted[next_file_index];
		dbg::out(dbg::info, "continue") << "The current file is: " << current_file->getPath()
				<< " which covers gens " << current_file->getFirstGen()
				<< " to " << current_file->getLastGen() << std::endl;

		//Determine the next file, skipping all files that don't expand upon our current file
		do{
			++next_file_index;
			if(next_file_index>=files_sorted.size()){
				next_file = 0;
				break;
			}
			next_file = &files_sorted[next_file_index];
		} while(current_file->getLastGen() >= next_file->getLastGen());

		//Determine the last line to write for the current file
		//The last line will end either at the first line of the next file,
		//or at the end of the current file if there is no next file
		int end_gen;
		if(next_file != 0){
		    if(current_file->getLastGen() >= next_file->getFirstGen()){
		        end_gen = next_file->getFirstGen()-1;
		    } else {
		        end_gen = current_file->getLastGen();
		    }
		} else {
			end_gen = current_file->getLastGen();
		}

		//Skip the header
		boost::filesystem::ifstream current_file_stream(current_file->getPath());
		if(current_file->hasHeader()){
			std::getline(current_file_stream, line);
			dbg::out(dbg::info, "continue") << "Skipping header: " << line << std::endl;
		}

		// Write lines until the start of the next file, or the end of this file if there is no next file
		int gen_nr = current_file->getFirstGen();
		do{
			if(std::getline(current_file_stream, line)){
			    gen_nr = getGen(line);
			    if(gen_nr <= end_gen){
			        output << line << '\n';
			    }
				dbg::out(dbg::info, "continue_full") << "Generation " << gen_nr << "/" << end_gen << ": " << line << std::endl;
			} else {
			    gen_nr = -1;
			}
		}
	    while(gen_nr < end_gen && gen_nr != -1);
		current_file_stream.close();

        if(gen_nr == -1){
            std::cerr << "Failed to read enough lines from: " << current_file->getPath() << " aborting merge." << std::endl;
            output.close();
            return;
        }
	}

	//Done writing temporary merge file
	output.close();

	//Delete partial files
	BOOST_FOREACH(boost::filesystem::path path, files){
		boost::filesystem::remove(path);
	}

	//Rename
	std::string merged_log_file = resultDirectory + "/" + filename;
	boost::filesystem::path merged_log_path(merged_log_file);
	boost::filesystem::rename(temp_log_path, merged_log_path);
}

}
}
#endif /* EXP_MODULARITY_STAT_MERGE_ONE_LINE_GEN_FILES_HPP_ */
