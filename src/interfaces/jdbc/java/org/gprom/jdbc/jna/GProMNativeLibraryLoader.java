/**
 * 
 */
package org.gprom.jdbc.jna;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.UUID;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.utility.LoggerUtil;
import org.gprom.jdbc.utility.OSInfo;
import org.gprom.jdbc.utility.OSInfo.OSArchType;
import org.gprom.jdbc.utility.OSInfo.OSType;


/**
 * @author lord_pretzel
 *
 */
public class GProMNativeLibraryLoader {

	Logger log = LogManager.getLogger(GProMNativeLibraryLoader.class);
	
	public static final GProMNativeLibraryLoader inst = new GProMNativeLibraryLoader();
	private static final String libraryName = "gprom";
        private static final String jnaPathProp = "jna.library.path";
        private static final Map<OSArchType,String> libextensions = new HashMap<OSArchType,String> ();
	private static final Map<OSArchType,String> folderName = new HashMap<OSArchType,String> ();
	private static final String relativeOffset = "";
	
	static {
		libextensions.put(OSArchType.Mac32, "dylib");
		libextensions.put(OSArchType.Mac64, "dylib");
		libextensions.put(OSArchType.Linux32, "so");
		libextensions.put(OSArchType.Linux64, "so");
		libextensions.put(OSArchType.Windows32, "dll");
		libextensions.put(OSArchType.Windows64, "dll");
		
		folderName.put(OSArchType.Mac32,  "libgpromnative/darwin_x32");
		folderName.put(OSArchType.Mac64,  "libgpromnative/darwin_x64");
		folderName.put(OSArchType.Linux32,  "libgpromnative/linux_x32");
		folderName.put(OSArchType.Linux64,  "libgpromnative/linux_x64");
		folderName.put(OSArchType.Windows32,  "libgpromnative/windows_x32");
		folderName.put(OSArchType.Windows64,  "libgpromnative/windows_x64");
	}
	
	
	private boolean isLoaded = false;
	private OSArchType os;
	private String tempDir;
	private File tempLibDir;
	private String libWithSuffix;
	private File libDir;
	private File libraryFile;
	
	
	public GProMNativeLibraryLoader () {
		
	}

        private void setJNALibPath(String dir) {
	    String previousPath = System.getProperty(jnaPathProp);
	    String newPath;
	    
	    if (previousPath == null || previousPath.trim().equals("")) {
		newPath = dir;
	    }
	    else {
		newPath = previousPath + ":" + dir;		
	    }
	    System.setProperty(jnaPathProp, newPath);
	    log.debug("have set {} to {}", jnaPathProp, newPath);
        }
    
	public synchronized boolean loadLibrary () throws IOException {
		if (!isLoaded) {
			detectOSAndTempDir();
			createTmpDirForLib();
			libWithSuffix = System.mapLibraryName(libraryName);
			log.debug("library to load: {}", libWithSuffix);
			
			try {
				String javaLibPath = System.getProperty("java.library.path");
				StringTokenizer libPaths = new StringTokenizer(javaLibPath, ":");
				
				while(libPaths.hasMoreTokens()) {
					String dir = libPaths.nextToken();
					libDir = new File(dir);				
					libraryFile = new File(libDir, libWithSuffix);
					if (libraryFile.exists()) {
						System.load(libraryFile.getAbsolutePath());
						log.info("successfully loaded library from {}", libraryFile.getAbsolutePath());
						isLoaded = true;
						setJNALibPath(dir);
						return true;
					}
					else
						log.debug("does not exist: " + libraryFile.getAbsolutePath());
				}
			} 
			catch (UnsatisfiedLinkError e) {
				LoggerUtil.logException(e, log);
				log.info("tried to load {} from system java.library.path {}", libWithSuffix, System.getProperty("java.library.path"));
			}
			
			try {
				InputStream in;
				String folder = relativeOffset + "/" + folderName.get(os);
				String resource = folder + "/" + libWithSuffix;
				
				// determine folder name
				log.debug("open input stream to {}/{}", folder, libWithSuffix);
				in = GProMNativeLibraryLoader.class.getResourceAsStream(resource);
				
				libraryFile = new File(tempLibDir, libWithSuffix);
				
				// set permissions
				libraryFile.setReadable(true);
				libraryFile.setWritable(true, true);
				libraryFile.setExecutable(true);

				// copy data
				Files.copy(in, libraryFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
				
				// delete file after exiting
				libraryFile.deleteOnExit();
				
				// load library
				log.debug("try to extract library to temporary dir {} and load it from there", libraryFile);
				System.load(libraryFile.getAbsolutePath());
				log.info("loaded library {} successfully", libWithSuffix);
				
				setJNALibPath(tempLibDir.getAbsolutePath());
				isLoaded = true;
			}
			catch (Exception e) {
				log.error("failed to extract library from jar file and load it");
				LoggerUtil.logException(e, log);
				if (libraryFile != null)
					libraryFile.delete();
				if (tempLibDir != null)
					tempLibDir.delete();
				throw(e);
			}
			finally {
				if (libraryFile != null)
					libraryFile.deleteOnExit();
			}
			return isLoaded;
		}
		return isLoaded;
	}
	
	/**
	 * @throws IOException 
	 * 
	 */
	private void createTmpDirForLib() throws IOException {
		tempLibDir = new File(tempDir, UUID.randomUUID().toString());
		if (!tempLibDir.mkdir())
			throw new IOException("was not able to create directory for library: " + tempLibDir.toString());
		tempLibDir.deleteOnExit();
	}

	/**
	 * 
	 */
	private void detectOSAndTempDir() {
		tempDir = System.getProperty("java.io.tmpdir");
		os = OSInfo.getOSArchType();
	}
	
}
