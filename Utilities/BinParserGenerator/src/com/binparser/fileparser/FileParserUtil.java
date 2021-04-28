package com.binparser.fileparser;

public class FileParserUtil {

	public static String getModifierValue(String mod){
		String value = "0";
		switch(Integer.valueOf(mod)){
			case 0: case 3: case 4:{
				value = "12000";
				break;
			}		
			case 6:{
				value = "128";
				break;
			}
		}
		return value;
	}
	
}
