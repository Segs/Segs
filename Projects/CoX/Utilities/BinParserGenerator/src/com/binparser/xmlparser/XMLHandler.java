package com.binparser.xmlparser;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.binparser.objects.KeyObject;
import com.binparser.objects.StructureObject;

public class XMLHandler {
	
	
	public static void writeXML(String path, StructureObject struct) throws ParserConfigurationException, TransformerException{

		DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
		DocumentBuilder docBuilder;
		docBuilder = docFactory.newDocumentBuilder();
		
		
		Document doc = docBuilder.newDocument();
		Element rootElement = doc.createElement("file");

		doc.appendChild(rootElement);
		Element el = doc.createElement("structure");
		rootElement.appendChild(el);
		el.setAttribute("name", struct.name);
		Iterator<KeyObject> iter = struct.list.iterator(); 

		while (iter.hasNext()){
			KeyObject o = iter.next(); 

			Element field = doc.createElement("field");
			el.appendChild(field);

			Attr attr1 = doc.createAttribute("name");
			attr1.setValue(o.name);
			field.setAttributeNode(attr1);

			Attr attr2 = doc.createAttribute("type");
			attr2.setValue(o.type);
			field.setAttributeNode(attr2);
			
			if(o.mod != null){
				Attr attr3 = doc.createAttribute("mod");
				attr3.setValue(o.mod);
				field.setAttributeNode(attr3);
			}
		}

		TransformerFactory transformerFactory = TransformerFactory.newInstance();
		Transformer transformer = transformerFactory.newTransformer();
		DOMSource source = new DOMSource(doc);
		StreamResult result = new StreamResult(new File(path));

		transformer.transform(source, result);

	
	}

	public static ArrayList<StructureObject> readXML(String path) throws ParserConfigurationException, SAXException, IOException{
		ArrayList<StructureObject> strucList = new ArrayList<StructureObject>();

		File fXmlFile = new File(path);
		DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
		DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
		Document doc = dBuilder.parse(fXmlFile);
		doc.getDocumentElement().normalize();

		NodeList nList = doc.getElementsByTagName("structure");
		
		for (int temp = 0; temp < nList.getLength(); temp++) {

			Node nNode = nList.item(temp);
			if (nNode.getNodeType() == Node.ELEMENT_NODE) {
				
				StructureObject strucObj = new StructureObject();
				Element eElement = (Element) nNode;
				strucObj.name = eElement.getAttribute("name");
				NodeList nList2 = eElement.getElementsByTagName("field");
				ArrayList<KeyObject> keyList = new ArrayList<KeyObject>();
				
				for(int temp2 = 0; temp2 < nList2.getLength(); temp2++){
					Node nNode2 = nList2.item(temp2);
					if (nNode2.getNodeType() == Node.ELEMENT_NODE) {
						if(nNode2.getNodeName() == "field"){
							KeyObject keyObj = new KeyObject();
							NamedNodeMap attrList = nNode2.getAttributes();
							
							for(int tmp3 = 0; tmp3 < attrList.getLength(); tmp3++){
								if(attrList.item(tmp3).getNodeName() == "name"){
									keyObj.name = attrList.item(tmp3).getNodeValue();
								}

								if(attrList.item(tmp3).getNodeName() == "type"){
									keyObj.type = attrList.item(tmp3).getNodeValue();
								}
								
								if(attrList.item(tmp3) != null){
									if(attrList.item(tmp3).getNodeName().equals("mod")){
										keyObj.mod = attrList.item(tmp3).getNodeValue();
									}
								}				
							}									
							if(keyObj.name != null && keyObj.type != null)
								keyList.add(keyObj);
						}
					}
					if(!keyList.isEmpty())
						strucObj.list = keyList;
				}
				if(!strucObj.list.isEmpty())
					strucList.add(strucObj);
			}
		}
		
		if(!strucList.isEmpty())
			return strucList;
		else 
			return null;
	}
	
}
