#!/usr/bin/env ruby
=begin
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2011 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
=end
$LOAD_PATH.unshift File.dirname($0)
EXE_EXT='.exe'
require 'open-uri'

require 'ui_updater'	
require 'project_handler'

class UpdaterForm < Qt::Dialog
	
    slots	'on_beginBtn_clicked()'
	slots	'on_projectSelector_currentIndexChanged(int)'
	slots	'on_componentSelector_currentIndexChanged(int)'
    def initialize(parent = nil)
        super(parent)
        @ui = Ui_Dialog.new
        @ui.setupUi(self)
        @ui.progressBar.value = 0
        @projects=nil
        @selected_manifest=""
        get_project_list("segs.nemerle.eu")
    end
    def on_beginBtn_clicked()
		proj_name = @ui.projectSelector.itemText(@ui.projectSelector.currentIndex)
		component_name = @ui.componentSelector.itemText(@ui.componentSelector.currentIndex)
        directory_to_check = "."
        directory_to_check = Qt::FileDialog.getExistingDirectory(self,
                                    tr("Please tell me where is #{proj_name}'s #{component_name} directory ?"),
                                    tr("."),
                                    Qt::FileDialog::DontResolveSymlinks | Qt::FileDialog::ShowDirsOnly)

        return if directory_to_check.nil? # no directory was selected

        if(false==download_mainfest("segs.nemerle.eu",directory_to_check))
			return
        end
        verify_manifest(directory_to_check,@selected_manifest)
    end
    def verify_manifest(dir_to_verify,manifest_file)
        out = `md5deep#{EXE_EXT} -l -r #{dir_to_verify} -n -x #{manifest_file}`
        @ui.debug_text.plainText=""
        if(not out.empty?)
            @ui.debug_text.appendHtml("<b>The following files failed verification check !</b><br>")
            @ui.debug_text.appendPlainText(out) 
        else
            @ui.debug_text.appendHtml("<b>All files verified</b>")
        end
    end
    def download_file(host,targetfile,filename)
		
        @ui.status_text.text = "Downloading #{filename}"
        begin
            file_length=1
            length_proc = lambda {|content_length| file_length=content_length}
            progress_proc = lambda {|size_so_far| @ui.progressBar.value = (100*size_so_far.to_f/file_length.to_f()).to_i}
    
            uri = URI.parse("http://#{host}/#{filename}")
            uri.open("r",:content_length_proc => length_proc, :progress_proc => progress_proc ) {|f|
                File.open(targetfile,"w") { |actual_file| actual_file << f.read } # copy received data
            }
        rescue Exception => e
            @ui.debug_text.appendHtml("<b>Exception thrown</b> "+e.message()+"<br>")
        end
		if( not File.exists?(targetfile))
			@ui.status_text.text += " .. failed"
			return false
		end
		@ui.status_text.text += " .. done"
		true
    end
    def get_project_list(host)
		fileName = "project_list.xml"
		if(false == download_file(host,fileName,"project_list.xml"))
			return
		end
	    handler = ProjectListHandler.new(self)
		reader = Qt::XmlSimpleReader.new
	    reader.contentHandler = handler
	    reader.errorHandler = handler

		file = Qt::File.new(fileName)
	    if !file.open(Qt::File::ReadOnly | Qt::File::Text)
	        Qt::MessageBox.warning(self, tr("Project list"),
	                             tr("Cannot read file %s:\n%s." %
	                             [fileName, file.errorString]))
	        return
	    end

	    xmlInputSource = Qt::XmlInputSource.new(file)
	    if (reader.parse(xmlInputSource, false))
		    @projects=handler.projects
			project_list_ready()
	    end
    end
    def project_list_ready()
		@ui.projectSelector.clear()
	    @projects.each {|proj|
			@ui.projectSelector.addItem(proj.name,Qt::Variant.fromValue(proj))
		}
	    if( not @projects.empty?)
			@ui.projectSelector.enabled = true
	    end
	    @ui.projectSelector.currentIndex=-1
    end
    def download_mainfest(host,directory_to_check)
		local_path=File.join(directory_to_check,@selected_manifest)
		return download_file(host,local_path,@selected_manifest)
    end
	def on_projectSelector_currentIndexChanged(idx)
		@ui.componentSelector.clear
		if(idx==-1 or @projects.nil? or @projects.empty?)
			@ui.componentBox.title="Some project's components"
			@ui.componentSelector.setEnabled(false)
			return
		end
		@ui.componentBox.title="#{@ui.projectSelector.itemText(idx)}'s components"
		@ui.componentSelector.setEnabled(true)
		dat = @ui.projectSelector.itemData(idx)
		sel_proj = dat.value
	    sel_proj.components.each {|name,manifest|
			@ui.componentSelector.addItem(name,Qt::Variant.new(manifest))
		}
	    @ui.componentSelector.currentIndex=0
	end
	def on_componentSelector_currentIndexChanged(idx)
		if(idx==-1 || @ui.componentSelector.count==0)
			@ui.beginBtn.setEnabled(false)
			return
		end
		@selected_manifest = @ui.componentSelector.itemData(idx).value
		if(@selected_manifest.nil? or @selected_manifest.empty?)
			return
		end
		@ui.beginBtn.setEnabled(true)
	end
end
