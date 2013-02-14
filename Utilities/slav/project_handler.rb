#!/usr/bin/env ruby
=begin
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2011 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
=end
$LOAD_PATH.unshift File.dirname($0)

class ProjectInfo
	attr_accessor :name
	attr_reader :components
	def initialize
		@name		= String.new
		@components = Hash.new
		@last_component=nil
	end
	def add_component(component_name)
		@components[component_name] = ""
		@last_component=component_name
	end
	def set_component_manifest(filename)
		@components[@last_component]=filename
	end
end
class ProjectListHandler < Qt::XmlDefaultHandler
	attr_reader :projects
	def initialize(my_ui)
		super()
		@currentText	= ""
		@m_dlg			= my_ui
		@state			= :initial
		@projects		= Array.new
		@current_project= nil
	end
	def startElement(namespaceURI, localName, qName, attributes)
		case @state
		when :initial
			if(localName!="segs")
				p "Bad xml format"
				return false
			end
			@state=:in_segs
		when :in_segs
			if(localName!="projects")
				p "Bad xml format"
				return false
			end
			@state=:in_projects
		when :in_projects
			if(@current_project.nil?)
				if(localName!="project" or attributes.count==0 or attributes.localName(0)!="name")
					p "Bad xml format: missing project entry"
					return false
				end
				@current_project = ProjectInfo.new
				@current_project.name=attributes.value(0).to_s
			elsif(localName=="component")
				@current_project.add_component(attributes.value(0).to_s)
			end
		end
		return true
	end
	def endElement(namespaceURI, localName, qName)
		if localName == "manifest"
			@current_project.set_component_manifest(@currentText)
		end
		if(localName == "project")
			@projects<<@current_project
			@current_project=nil
		end
		@currentText=""
		return true
	end
	def characters(str)
	    @currentText = str
	    return true
	end
	
	def fatalError(exception)
	    Qt::MessageBox.information(@m_dlg, Qt::Object.tr("Project list"),
	                             Qt::Object::tr("Parse error at line %s, column %s:\n%s" %
	                             [exception.lineNumber, exception.columnNumber,exception.message]))
	    return false
	end
	def errorString()
	    return @errorStr
	end
end
