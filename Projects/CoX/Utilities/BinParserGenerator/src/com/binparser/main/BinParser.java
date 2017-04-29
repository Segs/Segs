package com.binparser.main;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;

import org.xml.sax.SAXException;

import com.binparser.fileparser.FileParser;
import com.binparser.objects.KeyObject;
import com.binparser.objects.StructureObject;
import com.binparser.xmlparser.XMLHandler;

public class BinParser {

	/**
	 * @param args
	 * @throws TransformerException 
	 * @throws ParserConfigurationException 
	 * @throws IOException 
	 * @throws SAXException 
	 */
	public static void main(String[] args) throws ParserConfigurationException, TransformerException, SAXException, IOException {
	
		/*StructureObject structObj = new StructureObject();
		structObj.name = "SceneStorage";

		structObj.list = new ArrayList<KeyObject>();
		KeyObject kobj = new KeyObject("version", "unsigned int");
		structObj.list.add(kobj);
		kobj = new KeyObject("scenefile", "string");
		kobj.mod = "4";
		structObj.list.add(kobj);
		kobj = new KeyObject("defs", "array<Def>");
		structObj.list.add(kobj);
		kobj = new KeyObject("mods", "array<Def>");
		structObj.list.add(kobj);
		kobj = new KeyObject("refs", "array<Ref>");
		structObj.list.add(kobj);
		
		ArrayList<StructureObject> list1 = new ArrayList<StructureObject>();
		list1.add(structObj);
		
		XMLHandler.writeXML("C:\\test.xml", structObj);
	*/
		if(args.length > 0){
			
			//ArrayList<StructureObject> list = XMLHandler.readXML(args[0]);
			ArrayList<StructureObject> list = XMLHandler.readXML(args[0]);
			for(int i=0; i < list.size(); i++){
				String path = new File(".").getAbsolutePath();
				path = path.replace(".", "");
				FileParser.createClassFile(list.get(i), path);
				FileParser.createCppFile(list.get(i), path);
			}
		}
	}

}
