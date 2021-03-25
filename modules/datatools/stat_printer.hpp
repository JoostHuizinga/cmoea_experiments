/*
 * stat_printer.hpp
 *
 *  Created on: Nov 13, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_STAT_PRINTER_HPP_
#define MODULES_MISC_STAT_PRINTER_HPP_

namespace sferes
{
namespace stat
{

class StatPrinter{
public:
    void dataStart(){
        index = 0;
    }

    template<typename DataType>
    void add(bool active, std::string name, DataType value){
        if(!active) return;
        std::ostringstream ss;
        ss << value;
        std::string value_str = ss.str();
        if(index >= values.size()){
            headers.push_back(name);
            values.push_back(value_str);
        } else {
            values[index] = value_str;
        }
        ++index;
    }

    void dataPrint(boost::shared_ptr<std::ofstream> file_stream){
        if(file_stream->tellp() == 0){
            for(size_t i=0; i<headers.size(); ++i){
                (*file_stream) <<  headers[i] << " ";
            }
            (*file_stream) << "\n";
        }
        for(size_t i=0; i<values.size(); ++i){
            (*file_stream) <<  values[i] << " ";
        }
        (*file_stream) << std::endl;
    }

private:
    size_t index;
    std::vector<std::string> headers;
    std::vector<std::string> values;
};

}

}



#endif /* MODULES_MISC_STAT_PRINTER_HPP_ */
