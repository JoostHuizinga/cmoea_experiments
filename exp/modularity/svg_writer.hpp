/*
 * svg_writer.hpp
 *
 *  Created on: Dec 5, 2013
 *      Author: joost
 */

#ifndef SVG_WRITER_HPP_
#define SVG_WRITER_HPP_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/foreach.hpp>
#include <Eigen/Core>
#include <Eigen/LU>
#include <modules/datatools/position3.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sstream>

namespace sferes
{

template<typename ElementType>
bool contains(std::vector<ElementType> vector, ElementType element){
	for(size_t i=0; i<vector.size(); i++){
		if(vector[i] == element) return true;
	}
	return false;
}

template<typename NR>
inline std::string formatNr(NR number){
	std::stringstream ss;
	ss << "\"" << number << "\" ";
	return ss.str();
}

class SvgColor;
std::ostream& operator<<(std::ostream& is, const SvgColor& obj);

class SvgColor{
public:
	SvgColor(unsigned char r, unsigned char g, unsigned char b):
		r(r), g(g), b(b), none(false){
		//Do nothing
	}

	SvgColor():
		r(0), g(0), b(0), none(true){
		//Do nothing
	}

	std::string getString(){
		std::stringstream ss;
		ss << *this;
		return ss.str();
	}

	unsigned char r;
	unsigned char g;
	unsigned char b;
	bool none;
};

std::ostream& operator<<(std::ostream& is, const SvgColor& obj){
	if (obj.none){
		is << "none";
	}else {
		is << "#" << std::hex << std::setw(2) << std::setfill('0') << std::setw(2) << (int)obj.r << std::setw(2) << (int)obj.g << std::setw(2) << (int)obj.b << std::dec;
	}
	return is;
}


class SvgPoint2d{
public:
	SvgPoint2d(gen::spatial::Pos position3d):
		x(position3d.x()), y(position3d.y()){
		//Do nothing
	}

	SvgPoint2d(float x = 0, float y = 0):
		x(x), y(y){
		//Do nothing
	}

	float dist(const SvgPoint2d& other) const{
        float _x = x - other.x;
        float _y = y - other.y;
        return sqrt(_x * _x + _y * _y);
	}

	float x;
	float y;
};

// lexical order
template<typename VertexType>
struct sort_on_axis
{
	sort_on_axis(std::map<VertexType, SvgPoint2d> coordinate_map){
		_coordinate_map = coordinate_map;
	}

	bool operator() (const VertexType& i1, const VertexType& i2)
	{
		// first, compare the main objective
		if (_coordinate_map[i1].x > _coordinate_map[i2].x) return true;
		return false;
	}

protected:
	std::map<VertexType, SvgPoint2d> _coordinate_map;
};


template<typename VertexType>
class DepthLayer{
public:
	DepthLayer(){
		_first_member = true;
		_x_min = 0;
		_x_max = 0;
		_y_min = 0;
		_y_max = 0;
	}

	inline void add(const gen::spatial::Pos& member, VertexType vertex){
		_members.push_back(member);
		_vertices.push_back(vertex);
		if(_first_member){
			_x_min = member.x();
			_x_max = member.x();
			_y_min = member.y();
			_y_max = member.y();
			_first_member = false;
		} else{
			if(member.x() < _x_min) _x_min = member.x();
			if(member.x() > _x_max) _x_max = member.x();
			if(member.y() < _y_min) _y_min = member.y();
			if(member.y() > _y_max) _y_max = member.y();
		}
	}
	inline float getXMin() const{
		return  _x_min;
	}
	inline float getXMax() const{
		return _x_max;
	}
	inline float getYMin() const{
		return _y_min;
	}
	inline float getYMax() const{
		return _y_max;
	}

	inline float getWidth() const{
		return _x_max - _x_min;
	}

	inline float getHeight() const{
		return _y_max - _y_min;
	}

	inline size_t size() const{
		return _members.size();
	}

	inline gen::spatial::Pos& operator[](const size_t& index){
		return _members[index];
	}

	inline VertexType getVertex(const size_t& index){
		return _vertices[index];
	}

//	inline bool contains(VertexType& vertex){
//		return _vertices.count(vertex) > 0;
//	}

protected:
	bool _first_member;
	float _x_min;
	float _x_max;
	float _y_min;
	float _y_max;
	std::vector<gen::spatial::Pos> _members;
	std::vector<VertexType> _vertices;
};

template<typename VertexType>
class DepthLayer2d{
public:
	DepthLayer2d(){
		_first_member = true;
		_x_min = 0;
		_x_max = 0;
		_y_min = 0;
		_y_max = 0;
	}

	inline bool isMember(const VertexType& vertex) const{
		for(size_t i=0; i<_vertices.size(); i++){
			if(_vertices[i] == vertex) return true;
		}
		return false;
	}

	inline void merge(const DepthLayer2d<VertexType>& other, const float& threshold){
		//Add all vertices to this layer that were not already in this layer
		for(size_t i=0; i<other.size(); i++){
			if(this->isMember(other.getVertex(i)) == 0){
				this->add(other[i], other.getVertex(i));
//				_vertices.push_back(other.getVertex(i));
//				_members.push_back(other[i]);
			}
		}

		//Calculate the new center
		float center = 0;
		for(size_t i=0; i<this->size(); i++){
			center += _members[i].x;
		}
		center = center / (float) _members.size();

		//Redistribute nodes
		float width = threshold*(_members.size()-1);
		_x_min = center - width/2.0f;
		_x_max = center + width/2.0f;
		float x_current = _x_min;
		for(size_t i=0; i<this->size(); i++){
			_members[i].x = x_current;
			x_current+=threshold;
		}

	}

	/**
	 * Manhattan distance between two groups
	 */
	inline float dist(const DepthLayer2d<VertexType>& other) const{
		return distX(other) + distY(other);
	}

	inline float distX(const DepthLayer2d<VertexType>& other) const{
		if(other._x_min > _x_max) return other._x_min - _x_max;
		if(other._x_max < _x_min) return _x_min - other._x_max;
		return 0.0f;
	}

	inline float distY(const DepthLayer2d<VertexType>& other) const{
		if(other._y_min > _y_max) return other._y_min - _y_max;
		if(other._y_max < _y_min) return _y_min - other._y_max;
		return 0.0f;
	}

	//Keep group sorted
	inline void add(const SvgPoint2d& member, VertexType vertex){
		typename std::vector<SvgPoint2d>::iterator mem_it = _members.begin();
		typename std::vector<VertexType>::iterator vert_it = _vertices.begin();
		while(mem_it!=_members.end() && member.x > mem_it->x){
			mem_it++;
			vert_it++;
		}

		_members.insert(mem_it, member);
		_vertices.insert(vert_it, vertex);
		if(_first_member){
			_x_min = member.x;
			_x_max = member.x;
			_y_min = member.y;
			_y_max = member.y;
			_first_member = false;
		} else{
			if(member.x < _x_min) _x_min = member.x;
			if(member.x > _x_max) _x_max = member.x;
			if(member.y < _y_min) _y_min = member.y;
			if(member.y > _y_max) _y_max = member.y;
		}
	}
	inline float getXMin() const{
		return  _x_min;
	}
	inline float getXMax() const{
		return _x_max;
	}
	inline float getYMin() const{
		return _y_min;
	}
	inline float getYMax() const{
		return _y_max;
	}

	inline float getWidth() const{
		return _x_max - _x_min;
	}

	inline float getHeight() const{
		return _y_max - _y_min;
	}

	inline size_t size() const{
		return _members.size();
	}

	inline SvgPoint2d& operator[](const size_t& index){
		return _members[index];
	}

	inline const SvgPoint2d& operator[](const size_t& index) const{
		return _members[index];
	}

//	inline SvgPoint2d getMember(const size_t& index) const{
//		return _members[index];
//	}

	inline VertexType getVertex(const size_t& index) const{
		return _vertices[index];
	}

	inline bool operator==(const DepthLayer2d<VertexType>& other){
		for(size_t i=0; i<other.size(); i++){
			if(this->isMember(other.getVertex(i)) == 0){
				return false;
			}
		}
		return true;
	}

protected:
	bool _first_member;
	float _x_min;
	float _x_max;
	float _y_min;
	float _y_max;
	std::vector<SvgPoint2d> _members;
	std::vector<VertexType> _vertices;
};


class Style{
public:
	enum{fill = 0, fill_opacity, stroke, stroke_width, h_align, v_align, nr_of_arguments};
	Style(){
		style = std::vector<std::string> (nr_of_arguments);
	}

	void setFill(SvgColor color){
		std::stringstream ss;
		ss << "fill:"<< color;
		style[fill] = ss.str();
	}

	void setFillOpacity(float opacity){
		std::stringstream ss;
		ss << "fill-opacity:"<< opacity;
		style[fill_opacity] = ss.str();
	}

	void setStroke(SvgColor color){
		std::stringstream ss;
		ss << "stroke:"<< color;
		style[stroke] = ss.str();
	}

	void setStrokeWidth(float width){
		std::stringstream ss;
		ss << "stroke-width:"<< width;
		style[stroke_width] = ss.str();
	}

	std::string getStyleString() const{
		std::string result;
		result = "\"";
		bool previous = false;
		for(size_t i=0; i<style.size(); i++){
			if(style[i] != ""){
				if(previous) result+=";";
				result+=style[i];
				previous = true;
			}
		}
		result+="\"";
		return result;
	}

	//middle
	void setHAlign(std::string align){
	    style[h_align] = "text-anchor:" + align;
	    std::stringstream ss;
	}

	//central
    void setVAlign(std::string align){
        style[v_align] = "dominant-baseline:" + align;
    }

	std::vector<std::string> style;
};



class Marker{
public:
	Marker(){
		_url="";
	}

	std::string getUrl(){
		return _url;
	}

	  bool operator==(const Marker &other) const {
		  return other._url == _url;
	  }

protected:
    std::string _url;
};

class NoneMarker: public Marker{
	NoneMarker(){
		this->_url = "none";
	}

	void write(std::ostream stream){
		//nix
	}
};

class ArrowHead: public Marker{
	ArrowHead(){
		this->_url = "url(#head)";
	}

	void write(std::ostream stream){
		stream << "<marker id=" << formatNr("head") << "orient=" << formatNr("auto") << "markerWidth=" << formatNr("2") << "markerHeight=" << formatNr("4") << "refX=" << formatNr("0.1") << "refY=" << formatNr("2") << ">\n";
		stream << "<path d=" << formatNr("M0,0 V4 L2,2 Z") << "fill=" <<formatNr("red")<< "/>\n";
		stream << "</marker>\n";
	}
};


class ArrowHead2: public Marker{
	ArrowHead2(SvgColor color){
		_id = color.getString();
		_color = color;
		this->_url = "url(#" + _id + ")";
	}

	void write(std::ostream stream){
		stream << "<marker id=" << formatNr(_id) << " orient="<<formatNr("auto") << "refX="<<formatNr(0.0)<<" refY="<<formatNr(0.0)<<" style="<<formatNr("overflow:visible;")<<">\n";
		stream << "<path d="<<formatNr("M 0.0,0.0 L 5.0,-5.0 L -12.5,0.0 L 5.0,5.0 L 0.0,0.0 z")<< "\n";
		stream << "style=\"fill-rule:evenodd;fill:" << _color << ";stroke:" << _color << ";stroke-width:1.0pt;marker-start:none;\"\n";
		stream << "transform="<<formatNr("scale(0.4) rotate(180) translate(12.5,0)") << "/>\"\n";
		stream << "</marker>\n";
	}
protected:
    std::string _id;
	SvgColor _color;
};


std::ostream& operator<<(std::ostream& is, const Style& obj){
	is << obj.getStyleString();
	return is;
}


class SvgWriter{
protected:
	SvgColor _background;
	SvgColor _default_connection_color;
	SvgColor _inhib_color;
	SvgColor _excite_color;
	SvgColor _null_color;
	SvgColor _default_node_color;
	float _scale;
	float _side_margin;
	float _top_margin;
	float _default_line_width;
	float _min_line_width;
	float _uncover_threshold;
	float _default_shape_size;
	size_t _default_shape;
	int _x_min;
	int _x_max;
	int _y_min;
	int _y_max;
	int _width;
	int _height;
	bool _use_weight_width;
	bool _use_weight_color;
	bool _plot_bias;
	bool _uncover_neurons;
	bool _rectangles;
//	std::vector<std::vector<float> > _layers;
//	std::vector<float> _layer_pos;
	std::vector<SvgColor> _colors;
	std::vector<size_t> _shapes;
	std::vector<size_t> _switch_axis;
	std::vector<size_t> _source_depth;
	std::vector<size_t> _target_depth;
	std::vector<std::string> _labels;
	std::vector<float> _shape_sizes;
	std::string _default_label;

public:
	SvgWriter(){
		_scale = 50;
		_side_margin = 20;
		_top_margin = 20;
		_default_line_width = 1.5;
		_min_line_width = 0.5;
		_default_shape = 0;
		_uncover_threshold = 16.0f;
		_background = SvgColor(0, 0, 0);
		_default_node_color = SvgColor(0, 0, 0);
		_default_connection_color = SvgColor(0, 0, 0);
		_excite_color = SvgColor(0, 70, 0);
		_inhib_color = SvgColor(250, 32, 32);
		_null_color = SvgColor(100, 100, 100);
		_use_weight_width = true;
		_use_weight_color = true;
		_plot_bias = false;
		_uncover_neurons = true;
		_rectangles = false;
		_default_label = "";
		_default_shape_size = 10.0;

		_x_min = 0;
		_x_max = 0;
		_y_min = 0;
		_y_max = 0;
		_width = 0;
		_height = 0;

		_switch_axis.push_back(0);
		_switch_axis.push_back(1);
		_switch_axis.push_back(2);
	}

	inline void setDefaultShapeSize(float size){
	    _default_shape_size = size;
	}

	inline void setLabels(std::vector<std::string> labels){
	    _labels = labels;
	}

	inline void addSourceDepth(const size_t& depth){
		_source_depth.push_back(depth);
	}

	inline void clearSourceDepth(){
		_source_depth.clear();
	}

	inline void addTargetDepth(const size_t& depth){
		_target_depth.push_back(depth);
	}

	inline void clearTargetDepth(){
		_target_depth.clear();
	}

	inline void setAxis(const size_t& index, const size_t& value){
		_switch_axis[index] = value;
	}

	inline void switchAxis(const size_t& firstAxis, const size_t& secondAxis){
		size_t tmp = _switch_axis[firstAxis];
		_switch_axis[firstAxis] = _switch_axis[secondAxis];
		_switch_axis[secondAxis] = tmp;
	}

	inline void setColors(const std::vector<SvgColor>& colors){
		_colors = colors;
	}

	inline void setShapes(const std::vector<size_t>& shapes){
		_shapes = shapes;
	}

	template<typename G, typename VertexType>
	inline void setShapes(const G& g, const std::map<VertexType, size_t>& shapes){
		typedef typename std::map<VertexType, size_t>::const_iterator it_t;
		it_t it;

		BGL_FORALL_VERTICES_T(v, g, G)
		{
			it=shapes.find(v);
			if(it != shapes.end()){
				_shapes.push_back(it->second);
			} else {
				_shapes.push_back(_default_shape);
			}
		}
	}

	template<typename G>
	inline void setShapes(const G& g, const std::map<std::string, size_t>& shapes){
		typedef typename std::map<std::string, size_t>::const_iterator it_t;
		it_t it;

		BGL_FORALL_VERTICES_T(v, g, G)
		{
//			std::cout << "id: " << g[v].get_id() << std::endl;
			it=shapes.find(g[v].get_id());
			if(it != shapes.end()){
				_shapes.push_back(it->second);
			} else {
				_shapes.push_back(_default_shape);
			}
		}
	}


	inline void setScale(const float& scale){
		_scale = scale;
	}

	inline void setSideMargin(const float& margin){
		_side_margin = margin;
	}

	inline void setTopMargin(const float& margin){
		_top_margin = margin;
	}

	inline void setDefaultLineWidth(const float& width){
		_default_line_width = width;
	}

	inline void setDefaultShape(const size_t& shape){
		_default_shape = shape;
	}

	inline void setBackground(const SvgColor& color){
		_background = color;
	}

	inline void setDefaultNodeColor(const SvgColor& color){
		_default_node_color = color;
	}

	inline void setDefaultLineColor(const SvgColor& color){
		_default_connection_color = color;
	}

	inline void setExciteColor(const SvgColor& color){
		_excite_color = color;
	}

	inline void setInhibitColor(const SvgColor& color){
		_inhib_color = color;
	}

	inline void setWeightWidth(bool flag = true){
		_use_weight_width = flag;
	}

	inline void setWeightColor(bool flag = true){
		_use_weight_color = flag;
	}

	inline void setPlotBias(bool flag = true){
		_plot_bias = flag;
	}

	inline void setUncoverNeurons(bool flag = true){
		_uncover_neurons = flag;
	}

	inline void setRectangles(bool flag = true){
		_rectangles = flag;
	}

	inline void setScaledBoundaries(const float& x_min, const float& x_max, const float& y_min, const float& y_max){
		_x_min = x_min*_scale - _side_margin;
		_x_max = x_max*_scale + _side_margin;
		_y_min = y_min*_scale - _top_margin;
		_y_max = y_max*_scale + _top_margin;
		_width = _x_max - _x_min;
		_height = _y_max - _y_min;
	}

	inline void translate(std::ofstream& ofs, const int& x, const int& y){
		ofs << "<g transform=\"translate(" << x <<"," << y <<")\">" << "\n"; // WARNING translation
	}

	inline void drawRectangle(std::ofstream& ofs,
			const float& width,
			const float& height,
			const float& x_offset = 0.0,
			const float& y_offset = 0.0,
			Style style = Style()){
		ofs << "<rect ";
		ofs << "width="<< formatNr(width) ;
		ofs << "height=" << formatNr(height);
		ofs << "x=" << formatNr(x_offset);
		ofs << "y=" << formatNr(y_offset);
		ofs << "style=" << style;
		ofs << "/>\n";
	}

	inline void drawLine(std::ofstream& ofs, const SvgPoint2d& start, const SvgPoint2d& end, Style style = Style()){
		  ofs << "<line ";
		  ofs << "x1=" << formatNr(start.x);
		  ofs << "y1=" << formatNr(start.y);
          ofs << "x2=" << formatNr(end.x);
          ofs << "y2=" << formatNr(end.y);
		  ofs << "style=" << style;
		  ofs << "/>\n";
	}

	inline void drawCircle(std::ofstream& ofs, const SvgPoint2d& center, const float& radius = 10, Style style = Style()){
		  ofs << "<circle ";
		  ofs << "cx=" << formatNr(center.x);
		  ofs << "cy=" << formatNr(center.y);
		  ofs << "r=" << formatNr(radius);
		  ofs << "style=" << style;
		  ofs << "/>\n";
	}

	inline void drawText(std::ofstream& ofs, const SvgPoint2d& center, std::string text, Style style = Style()){
	    ofs << "<text ";
	    ofs << "x=" << formatNr(center.x);
	    ofs << "y=" << formatNr(center.y);
	    ofs << "style=" << style;
	    ofs << ">";
	    ofs << text;
	    ofs << "</text>\n";
	}

	inline void writeHeader(std::ofstream& ofs, const int& width,const int& height){
		  ofs << "<?xml version=\"1.0\" standalone=\"no\"?>" << "\n";
		  ofs << "<svg width=\"" << width <<"\" height=\"" << height <<"\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">" << "\n";
	}

	inline void writeFooter(std::ofstream& ofs){
		  ofs << "</svg>" << std::endl;
	}


	/*
	 * Uncover neurons in layers
	 */
	inline void uncoverNeurons(std::vector<std::vector<float> >& layers, float threshold = 0.2){
		bool uncovered = false;

		while(!uncovered){
	      uncovered = true;
		  for (size_t i = 0; i < layers.size(); ++i){
			  for (size_t j = 0; j < layers[i].size(); ++j){
				  for (size_t k = 0; k < layers[i].size(); ++k){
					  if (j != k && fabs(layers[i][j] - layers[i][k]) < threshold){
						  layers[i][j] += threshold;
						  layers[i][k] -= threshold;
						  uncovered = false;
					  }
				  }
			  }
		  }
		}
	}

	/**
	 * Uncovers neurons based on a coordsmap
	 */
	template<typename G, typename VertexType>
	inline void uncoverNeurons(const G& g, std::map<VertexType, SvgPoint2d>& coordinate_map, float threshold = 4.0){
		bool uncovered = false;

		SvgPoint2d position1;
		SvgPoint2d position2;

		std::map<VertexType, DepthLayer2d<VertexType> > group;

		BGL_FORALL_VERTICES_T(v1, g, G){
			group[v1].add(coordinate_map[v1], v1);
		}

		while(!uncovered){
			uncovered = true;
			BGL_FORALL_VERTICES_T(v1, g, G){
//				position1 = coordinate_map[v1];
				BGL_FORALL_VERTICES_T(v2, g, G){
					if(group[v1] == group[v2]) continue;
//					position2 = coordinate_map[v2];

					if(group[v1].dist(group[v2]) < threshold){
						group[v1].merge(group[v2], threshold);
						group[v2] = group[v1];
						for(size_t i=0; i<group[v1].size();i++){
							coordinate_map[group[v1].getVertex(i)] = group[v1][i];
						}

//						std::cout << coordinate_map[v1].x << " " << coordinate_map[v2].x << std::endl;
//						if(coordinate_map[v1].x > coordinate_map[v2].x){
//							coordinate_map[v1].x += threshold/2.0;
//							coordinate_map[v2].x -= threshold/2.0;
//						} else {
//							coordinate_map[v1].x -= threshold/2.0;
//							coordinate_map[v2].x += threshold/2.0;
//						}
						uncovered = false;
					}
				}
			}
		}
	}

	inline void drawShape(std::ofstream& ofs, int shape, const SvgPoint2d& center, const SvgColor& fill_color, const float& size = 10.0){
		Style style;
		style.setFill(fill_color);

		 switch(shape){
		 case 0:{
			 drawCircle(ofs, center, size, style);
			 break;
		 }
		 case 1:{
			 style.setStrokeWidth(5.0);
			 style.setStroke(SvgColor(170, 0, 240));
			 drawCircle(ofs, center, size, style);
			 break;
		 }
		 case 2:{
			 style.setStrokeWidth(5.0);
			 style.setStroke(SvgColor(240, 170, 0));
			 drawCircle(ofs, center, size, style);
			 break;
		 }
		 case 3:{
			 style.setStrokeWidth(5.0);
			 style.setStroke(SvgColor(50, 50, 50));
			 drawCircle(ofs, center, size, style);
			 break;
		 }
		 case 4:{
			 style.setStrokeWidth(1.0);
			 style.setStroke(SvgColor(0, 0, 0));
			 drawCircle(ofs, center, size, style);
			 break;
		 }
		 default:
			 std::cout << "Unknown shape: " << shape << std::endl;
		 }
	}

	inline SvgColor& getColor(const int& i){
		if(i < _colors.size()){
			return _colors[i];
		} else {
			return _default_node_color;
		}
	}

	inline size_t& getShape(const int& i){
		if(i < _shapes.size()){
			return _shapes[i];
		} else {
			return _default_shape;
		}
	}

	inline std::string& getLabel(const int& i){
	    if(i < _labels.size()){
	        return _labels[i];
	    } else {
	        return _default_label;
	    }
	}

    inline float& getShapeSize(const int& i){
        if(i < _shape_sizes.size()){
            return _shape_sizes[i];
        } else {
            return _default_shape_size;
        }
    }

	inline SvgColor& getWeightDependentColor(const float& weight){
		if(weight > 0.0){
			return _excite_color;
		} else if (weight < 0.0) {
			return _inhib_color;
		} else {
			return _null_color;
		}
	}

	template<typename VertexType>
	std::map<VertexType, gen::spatial::Pos> switch_axis(const std::map<VertexType, gen::spatial::Pos>& coordinate_map){
		std::map<VertexType, gen::spatial::Pos> new_map;
		dbg::out(dbg::info, "visualization_coords") << "Switch axis: " << _switch_axis[0] << " " << _switch_axis[1] << " " << _switch_axis[2] <<std::endl;
		for(typename std::map<VertexType, gen::spatial::Pos> ::const_iterator iter = coordinate_map.begin(); iter != coordinate_map.end(); ++iter)
		{
			gen::spatial::Pos old = iter->second;
			dbg::out(dbg::info, "visualization_coords") << "Before switch x: " << old.x() << " y: " << old.y() << " z: " << old.z() <<std::endl;
			new_map[iter->first] = gen::spatial::Pos(old[_switch_axis[0]], old[_switch_axis[1]], old[_switch_axis[2]]);
		}
		return new_map;
	}

	template<typename VertexType>
	void set_extremes(std::map<float, DepthLayer<VertexType> > LayerMap){
		float currentWidth = 0;
		float currentHeight = 0;
		float y_min = std::numeric_limits<float>::max();
		float y_max = std::numeric_limits<float>::min();
		for(typename std::map<float, DepthLayer<VertexType> >::iterator iter = LayerMap.begin(); iter != LayerMap.end(); ++iter)
		{
			DepthLayer<VertexType> layer = iter->second;
			currentWidth += layer.getWidth()*_scale + _side_margin*2;
			if((layer.getHeight()*_scale + + _top_margin*2) > currentHeight) currentHeight = (layer.getHeight()*_scale + + _top_margin*2);
			if(layer.getYMin() < y_min) y_min = layer.getYMin();
			if(layer.getYMax() > y_max) y_max = layer.getYMax();
		}

		_y_min = y_min*_scale - _top_margin;
		_y_max = y_max*_scale + _top_margin;
		_width = currentWidth;
		_height = currentHeight;
	}

	template<typename G, typename VertexType>
	inline void to_svg(const G& g, const std::map<VertexType, gen::spatial::Pos>& coordinate_map, std::ofstream& ofs)
	{
		size_t shape = _default_shape;
		SvgColor color = _default_connection_color;
		float width = _default_line_width;
		float size = _default_shape_size;
		std::string label = "";
		typedef std::map<float, DepthLayer<VertexType> > depth_map_t;
		typedef std::map<VertexType, size_t> vertex_depth_map_t;
		std::map<VertexType, gen::spatial::Pos> switched_map = switch_axis(coordinate_map);

		std::map<VertexType, SvgPoint2d> new_pos_map;
		std::vector<VertexType> drawingOrder;
		std::map<VertexType, int> originalIndex;

		vertex_depth_map_t vertex_depth_map;

		//Make a depth map
		depth_map_t  map_per_depth;
		std::vector<float> depths;
		std::vector<float> rectangles;
		BGL_FORALL_VERTICES_T(v, g, G)
		{
			dbg::out(dbg::info, "visualization_coords") << "After switch x: " << switched_map[v].x() << " y: " << switched_map[v].y() << " z: " << switched_map[v].z() <<std::endl;
			float z = switched_map[v].z();
			if(!map_per_depth.count(z)){
				map_per_depth[z] = DepthLayer<VertexType>();
			}
			map_per_depth[z].add(switched_map[v], v);
		}

		//Calculate the width, height, global y_min and global y_max
		set_extremes(map_per_depth);

		//Sort according to depth
		for(typename depth_map_t::iterator iter = map_per_depth.begin(); iter != map_per_depth.end(); ++iter)
		{
			depths.push_back(iter->first);
		}
		std::sort(depths.begin(), depths.end(), std::greater<float>());

		//For each depth
		float currentOffset = _side_margin;
		for(size_t i=0; i<depths.size();i++){
			DepthLayer<VertexType> layer = map_per_depth[depths[i]];
			float layer_width = layer.getWidth()*_scale + _side_margin*2;

			for(size_t j=0; j<layer.size(); j++){
				vertex_depth_map[layer.getVertex(j)] = i;
				new_pos_map[layer.getVertex(j)] = SvgPoint2d(layer[j]*_scale + gen::spatial::Pos(currentOffset - layer.getXMin()*_scale, -_y_min, 0.0));
			}

			currentOffset += layer_width;
			rectangles.push_back(layer_width);
		}



		if(_uncover_neurons) uncoverNeurons(g, new_pos_map, _uncover_threshold);

		writeHeader(ofs, _width, _height);
		Style backgroundStyle;
		backgroundStyle.setFill(_background);
		drawRectangle(ofs, _width, _height, 0, 0, backgroundStyle);

		Style rectangleStyle;
		rectangleStyle.setFill(SvgColor());
		rectangleStyle.setStroke(SvgColor(0,0,0));

		float rect_margin_side = _side_margin/3;
		float rect_margin_top = _top_margin/3;


		if(_rectangles){
			currentOffset = rect_margin_side;
			for(size_t i =0;i < rectangles.size(); i++){
				drawRectangle(ofs, rectangles[i] - rect_margin_side*2, _height - rect_margin_top*2, currentOffset, rect_margin_top, rectangleStyle);
				currentOffset += rectangles[i];
			}
		}

		//Draw connections
		BGL_FORALL_EDGES_T(e, g, G)
		{
			VertexType src = boost::source(e, g);
			VertexType tgt = boost::target(e, g);
			if(!_source_depth.empty() && !contains(_source_depth, vertex_depth_map[src])) continue;
			if(!_target_depth.empty() && !contains(_target_depth, vertex_depth_map[tgt])) continue;
			if(_use_weight_width) width = fmax((fabs(g[e].get_weight().data(0)))*3, _min_line_width);
			if(_use_weight_color) color = getWeightDependentColor(g[e].get_weight().data(0));

			Style lineStyle;
			lineStyle.setStroke(color);
			lineStyle.setStrokeWidth(width);

			drawLine(ofs, new_pos_map[src], new_pos_map[tgt], lineStyle);
		}

		//Draw neurons

		int k = 0;
		BGL_FORALL_VERTICES_T(v, g, G){
			drawingOrder.push_back(v);
			originalIndex[v] = k;
			k++;
		}

		std::sort(drawingOrder.begin(), drawingOrder.end(), sort_on_axis<VertexType>(new_pos_map));

		for(int j=0; j<drawingOrder.size(); j++)
		{
			color = getColor(originalIndex[drawingOrder[j]]);
			shape = getShape(originalIndex[drawingOrder[j]]);
			label = getLabel(originalIndex[drawingOrder[j]]);
			size = getShapeSize(originalIndex[drawingOrder[j]]);
			drawShape(ofs, shape, new_pos_map[drawingOrder[j]], color, size);

			if(_plot_bias){
				width = fabs(float(g[drawingOrder[j]].get_afparams().data(0)))*3.f + 1.f;
				color = getWeightDependentColor(g[drawingOrder[j]].get_afparams().data(0));
				drawShape(ofs, shape, new_pos_map[drawingOrder[j]], color, width);
			}

			if(label != ""){
			    Style textStyle;
			    textStyle.setHAlign("middle");
			    textStyle.setVAlign("central");
			    textStyle.setFill(SvgColor(255,255,255));
			    drawText(ofs, new_pos_map[drawingOrder[j]], label, textStyle);
			}
		}
		writeFooter(ofs);
	}
};
}

#endif /* SVG_WRITER_HPP_ */
