package com.binparser.fileparser;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Iterator;

import com.binparser.objects.KeyObject;
import com.binparser.objects.StructureObject;

public class FileParser {

	public static void createClassFile(StructureObject struc, String path){
		try{
			FileWriter fstream = new FileWriter(path + struc.name + ".h");
			BufferedWriter out = new BufferedWriter(fstream);
			out.write("#include \"ReadableStructures.h\"");
			out.newLine();
			out.newLine();
			out.write("class Def;");
			out.newLine();
			out.write("class Ref;");
			out.newLine();
			out.newLine();
			out.write("class " + struc.name + " : public BinReadable");
			out.newLine();
			out.write("{");
			out.newLine();
			out.write("public:");
			out.newLine();
			
			Iterator<KeyObject> iter = struc.list.iterator();
			
			while(iter.hasNext()){
				KeyObject obj = iter.next();
				if(obj.type.equals("string")){
					out.write("	std::" + obj.type + " " + obj.name + ";");
				}else if(obj.type.contains("array")){
					String[] tmpStr = obj.type.split("<");
					String tmpType = tmpStr[1].replace(">", "");
					
					out.write("	std::" + "vector<"+ tmpType + " *> " + obj.name + ";");
				}else{
					out.write("	" + obj.type + " " + obj.name + ";");
				}
		
				out.newLine();
			}
			out.newLine();
			out.write("	int serializefrom(IStore *);");
			out.newLine();
			out.write("	static int serializeArrayfrom(IStore *);");
			out.newLine();
			out.write("};");
			out.close();
		}catch (Exception e){
			System.err.println("Error: " + e.getMessage());
		}
	}
	
	public static void createCppFile(StructureObject struc, String path){
		try{
			FileWriter fstream = new FileWriter(path + struc.name + ".cpp");
			BufferedWriter out = new BufferedWriter(fstream);
			out.write("#include \"" + struc.name + ".h\"");
			out.newLine();
			out.write("#include \"Def.h\"");
			out.newLine();
			out.write("#include \"Ref.h\"");
			out.newLine();
			out.newLine();
			out.write("int " + struc.name + "::serializefrom(IStore *source)");
			out.newLine();
			out.write("{");
			out.newLine();
			out.write("	source->read(version);");
			
			Iterator<KeyObject> iter = struc.list.iterator();
			
			while(iter.hasNext()){
				KeyObject obj = iter.next();
				if(obj.type.contains("array")){
					String[] tmpSplit = obj.type.split("<");
					String tmpType = tmpSplit[1].replace(">", "");
					out.write("	" + tmpType + "::serializeArrayfrom(source," + obj.name + ");    ");
				}else if(obj.mod != null && obj.type.equals("string")){
					out.write("	int " + "max_" + obj.name + "_length = " + FileParserUtil.getModifierValue(obj.mod) + ";");
					out.newLine();
					out.write("	" + obj.name + " = source->read_str(max_" + obj.name + "_length);");
				}
		
				out.newLine();
			}
			out.write("}");
			out.newLine();
			out.write("int " + struc.name + "::serializeArrayfrom(IStore *source,std::vector<" + struc.name + " *> &tgt)");
			out.newLine();
			out.write("{");
			out.newLine();
			out.write("	return -1;");
			out.newLine();
			out.write("}");
			out.close();
		}catch (Exception e){
			System.err.println("Error: " + e.getMessage());
		}
	}
}


