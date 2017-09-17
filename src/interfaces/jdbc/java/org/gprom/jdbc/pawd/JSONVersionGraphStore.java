package org.gprom.jdbc.pawd;

import org.codehaus.jackson.JsonGenerationException;
import org.codehaus.jackson.map.JsonMappingException;
import org.codehaus.jackson.map.ObjectMapper;

import java.io.File;
import java.io.IOException;
import java.util.Properties;




/**
 * @author Amer
 *
 */
public class JSONVersionGraphStore implements VersionGraphStore {

	private static final String PROP_STORE_DIR = "dir";
	
	private File storeDir;

	

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#initialize(java.util.Properties)
	 */
	@Override
	public void initialize(Properties options) throws IllegalArgumentException {
		if (options.getProperty(PROP_STORE_DIR) == null)
			throw new IllegalArgumentException ("need to provide option " + PROP_STORE_DIR);
		storeDir = new File(options.getProperty(PROP_STORE_DIR));
		if (!storeDir.isDirectory())
			throw new IllegalArgumentException ("dir " + storeDir + " does not exist");
	}
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#load(java.lang.String)
	 */
	@Override
	public VersionGraph load(String versionGraphId) {
		File graphFile = new File(storeDir, createFileName(versionGraphId));
		ObjectMapper mapper = new ObjectMapper();
		VersionGraph LoadedGraph = null;
		try {
			// Convert JSON string from file to Object
			LoadedGraph = mapper.readValue(graphFile, VersionGraph.class);
			System.out.println("Converted JSON string from file to Object");
		} catch (JsonGenerationException e) {
			e.printStackTrace();
		} catch (JsonMappingException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return LoadedGraph;
	}
	/**
	 * @param versionGraphId
	 * @return
	 */
	private String createFileName(String versionGraphId) {
		return versionGraphId + ".json";
	}
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.pawd.VersionGraphStore#save(org.gprom.jdbc.pawd.VersionGraph)
	 */
	@Override
	public void save(VersionGraph g) {
		ObjectMapper mapper = new ObjectMapper();
		try {
			//Convert object to JSON string and save into file directly
			mapper.writeValue(new File(storeDir +"\\"+ createFileName(g.getId())), g);
			System.out.println("Saved VersionGraph to file");
		} catch (JsonGenerationException e) {
			e.printStackTrace();
		} catch (JsonMappingException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}




}
