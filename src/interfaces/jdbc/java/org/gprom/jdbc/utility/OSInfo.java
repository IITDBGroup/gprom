/**
 * 
 */
package org.gprom.jdbc.utility;

import java.util.Locale;

/**
 * @author lord_pretzel
 *
 */
public class OSInfo {

	public enum OSArchType {
		Windows64,
		Windows32,
		Mac32,
		Mac64,
		Linux64,
		Linux32,
		Other
	}
	
	public enum OSType {
		Windows,
		Mac,
		Linux,
		Other
	}
	
	public static OSType getOSType () {
		String os = System.getProperty("os.name");
        if (os.contains("Windows")) {
            return OSType.Windows;
        }
        else if (os.contains("Mac") || os.contains("Darwin")) {
            return OSType.Mac;
        }
        else if (os.contains("Linux")) {
            return OSType.Linux;
        }		
        return OSType.Other;
	}
	
	public static OSArchType getOSArchType () {
		String arch = System.getProperty("os.arch").toLowerCase(Locale.US);
		OSType os = getOSType();
		
		switch (os) {
			case Windows: {
				if (arch.contains("64"))
					return OSArchType.Windows64;
				else
					return OSArchType.Windows32;			
			}
			case Mac: {
				if (arch.contains("64") || arch.contains("universal"))
					return OSArchType.Mac64;
				else
					return OSArchType.Mac32;
			}
			case Linux: {
				if (arch.contains("64"))
					return OSArchType.Linux64;
				else
					return OSArchType.Linux32;			
			}
			default:
				return OSArchType.Other;
		}
	}
	
}
