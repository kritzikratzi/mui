//
//  MuiParameterPanel.h
//  Sorry, deprecated! 
//  MuiEditorTest
//
//  Created by Hansi on 30.04.16.
//
//

#ifndef __MuiEditorTest__MuiParameterPanel__
#define __MuiEditorTest__MuiParameterPanel__

#include "MuiCore.h"
#include "Label.h"
#include "SliderWithLabel.h"
#include "ToggleButton.h"
#include "MuiTextArea.h"
#include <typeindex>
#include <unordered_map>

namespace mui{
	
	
	
	
	
	class ParameterPanel : public Container{
	public:
		class Section;
		template<typename MuiType, typename DataType>
		class Row;
		
		ParameterPanel(string title = "");
		virtual ~ParameterPanel();
		
		string getTitle() const;
		void setTitle( string text );
		
		string getSectionTitle( string sectionId );
		void setSectionTitle( string sectionId, string titel );

		Section * addSection( string sectionId );
		Section * addSection( string sectionId, string label );
		Section * getSection( string sectionId );
		Section * getCurrentSection();
		Section * getOrCreateSection( string sectionId, string label );
		
		mui::ParameterPanel::Row<mui::SliderWithLabel, float> * addSliderInt(string title, int min, int max, int value) {
			SliderWithLabel * slider = new SliderWithLabel(0, 0, 100, 20, min, max, value, 0);
			slider->label->fg = labelFg;
			mui::ParameterPanel::Row<mui::SliderWithLabel,float> * row = new mui::ParameterPanel::Row<SliderWithLabel, float>(this, title, NULL, slider);

			getCurrentSection()->addRow<mui::SliderWithLabel,float>(row);
			rows.insert(pair<string, data::Attribute>(title, row));
			return row;
		}
		Row<SliderWithLabel,float> * addSlider( string label, float min, float max, float value, int decimalDigits = 2 );
		mui::ParameterPanel::Row<ToggleButton, bool> * addToggle(string label, bool selected) {
			ToggleButton * button = new ToggleButton(label);
			button->selected = selected;
			button->label->horizontalAlign = Left;
			button->fg = labelFg;
			button->bg = ofColor(0, 0);
			button->selectedBg = ofColor(0, 0);
			button->selectedFg = labelFg;
			button->checkbox = true;
			button->checkboxAlign = Right;

			mui::ParameterPanel::Row<ToggleButton, bool> * row = new mui::ParameterPanel::Row<ToggleButton, bool>(this, label, button->label, button);

			getCurrentSection()->addRow(row);
			rows.insert(pair<string, data::Attribute>(label, row));
			return row;
		}
		Row<TextArea,string> * addText( string label, string text );
		
		mui::ParameterPanel::Row<Container, bool> * addContainer(string label, mui::Container * container) {
			auto row = new mui::ParameterPanel::Row<Container, bool>(this, label, NULL, container);

			getCurrentSection()->addRow(row);
			rows.insert(pair<string, data::Attribute>(label, row));
			return row;
		}
		
		
		void setLabelColumnWidth( float width ); 
		
		bool getBool( string rowId );
		void setBool( string rowId, bool value );
		int getInt( string rowId );
		void setInt( string rowId, int value );
		float getFloat( string rowId );
		void setFloat( string rowId, float value );
		string getString( string rowId );
		void setString( string rowId, string value );
		
		void layout();
		
		
		template<typename MuiType, typename DataType>
		void registerGetter( std::function<DataType(MuiType*)> func ){
			getters.insert(std::pair<std::type_index,data::Attribute>(std::type_index(typeid(Row<MuiType,DataType>*)), func));
		};
		
		template<typename MuiType, typename DataType>
		void registerSetter( std::function<void(MuiType*, DataType & type)> func ){
			setters.insert(std::pair<std::type_index,data::Attribute>(std::type_index(typeid(Row<MuiType,DataType>*)), func));
		};
		
		template<typename T>
		T getValue( string rowId ){
			std::unordered_map<string,mui::data::Attribute>::iterator it = rows.find(rowId);
			if( it != rows.end() ){
				mui::data::Attribute &attr = it->second;
				Row<Container,T> * row = attr.value_unsafe<Row<Container,T>*>();
				
				auto getter = getters.find(std::type_index(attr.type()));
				if( getter != getters.end() ){
					auto & func = getter->second.value_unsafe<std::function<T(Container*)>>();
					return func(row->control);
				}
				else{
					ofLogError()
					<< "ofxMightyUI in getValue<T>('" << rowId << "'): no getter for row '" << rowId << "'. You can register a custom one with " << endl
					<< "parameterPanel.registerGetter<MuiClass,DataType>([](MuiClass * obj){ return obj->myVal; }); ?" << endl;
				}
			}
			else{
				ofLogError() << "ofxMightyUI in getValue<T>('" << rowId << "'): No row with this name was found. " << endl;
			}
			
			return T();
		}
		
		template<typename T>
		void setValue( string rowId, const T &value ){
			std::unordered_map<string,mui::data::Attribute>::iterator it = rows.find(rowId);
			if( it != rows.end() ){
				mui::data::Attribute &attr = it->second;
				Row<Container,T> * row = attr.value_unsafe<Row<Container,T>*>();
				
				auto setter = setters.find(std::type_index(attr.type()));
				if( setter != setters.end() ){
					auto & func = setter->second.value_unsafe<std::function<void(Container*,const T&)>>();
					func(row->control, value);
					return;
				}
				else{
					ofLogError()
					<< "ofxMightyUI in setValue<T>('" << rowId << "'): no getter for row '" << rowId << "'. You can register a custom one with " << endl
					<< "parameterPanel.registerGetter<TargetClass,float>([](TargetClass * obj){ return obj->myVal; }); ?" << endl;
				}
			}
			else{
				ofLogError() << "ofxMightyUI in setValue<T>('" << rowId << "'): No row with this name was found. " << endl;
			}
			
			return;
		}
		
	protected:
		std::unordered_map<string,data::Attribute> rows;
		std::unordered_map<std::type_index,data::Attribute> getters;
		std::unordered_map<std::type_index,data::Attribute> setters;
		vector<Section*> sections;
		Section * currentSection;
		float labelColumnWidth;
		ofColor labelFg; 
		
	public:
		Label * titleLabel;
		
		template<typename MuiType, typename DataType>
		class Row : public Container{
		public: 
			ParameterPanel * panel;
			Label * titleLabel;
			MuiType * control;
			bool customLabel;
			
			Row( ParameterPanel * panel, string title, Label * existingLabel, MuiType * control ) : Container( 0,0, 100, 20), control(control), panel(panel){
				ignoreEvents = true; 
				if( existingLabel == NULL ){
					titleLabel = new Label(title,0,0,70,20);
					add(titleLabel);
					customLabel = false;
				}
				else{
					titleLabel = existingLabel;
					customLabel = true;
				}
				add(control);
			}
			
			virtual ~Row(){
				if(!customLabel) delete titleLabel;
			}
			
			void layout(){
				if( customLabel ){
					control->x = 0;
					control->width = width;
				}
				else{
					titleLabel->width = panel->labelColumnWidth;
					control->x = titleLabel->width + 2;
					control->width = width - titleLabel->width - 2;
				}
			}
			
			string getTitle(){
				return titleLabel->text;
			}
			
			void setTitle( string title ){
				titleLabel->text = title;
				titleLabel->commit();
			}
			
			void touchDown( ofTouchEventArgs &args ){
				control->requestFocus(args);
			}
		};
		
		class Section : public Container{
		public: 
			Label * titleLabel;
			string sectionId;
			
			Section( string sectionId, string title ) : Container( 0, 0, 100, 100 ), sectionId(sectionId){
				titleLabel = new Label(title,0,0,60,20);
				ignoreEvents = true;
				add(titleLabel);
			}
			
			virtual ~Section(){
				delete titleLabel;
			}
			
			vector<mui::data::Attribute> rows;
			
			void layout(){
				float yy;
				if( titleLabel->text == "" ){
					yy = 0;
				}
				else{
					titleLabel->width = width;
					yy = titleLabel->height;
				}
				for( mui::Container * c : children ){
					if( c == titleLabel ) continue;
					if( !c->visible ) continue; 
					c->y = yy;
					c->x = 5;
					c->width = width-10;
					yy += c->height;
					c->handleLayout(); 
				}
				
				this->height = yy;
			}
			
			template<typename MuiType,typename DataType>
			void addRow( mui::ParameterPanel::Row<MuiType,DataType> * row ){
				rows.push_back(row);
				add(row);
			}
			
			void sort( std::function<bool(string,string)> comp ){
				std::sort(children.begin(), children.end(), [&](mui::Container * a, mui::Container * b){
					if(a == titleLabel ) return true;
					else if( b == titleLabel ) return false;
					else return comp(((Row<mui::Container,void*>*)a)->titleLabel->text, ((Row<mui::Container,void*>*)b)->titleLabel->text);
				});
			}
			
			string getTitle(){
				return titleLabel->text;
			}
			
			void setTitle( string title ){
				titleLabel->text = title;
				titleLabel->commit();
			}
		};
	};
};

#endif /* defined(__MuiEditorTest__MuiParameterPanel__) */
