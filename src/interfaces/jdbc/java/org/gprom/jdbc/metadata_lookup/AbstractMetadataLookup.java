/**
 * 
 */
package org.gprom.jdbc.metadata_lookup;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;

import org.apache.log4j.Logger;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionClose_callback;
import org.gprom.jdbc.jna.GProM_JNA.GProMMetadataLookupPlugin.databaseConnectionOpen_callback;
import org.gprom.jdbc.metadata_lookup.postgres.PostgresCatalogLookup;
import org.gprom.jdbc.utility.LoggerUtil;

/**
 * @author lord_pretzel
 *
 */
public class AbstractMetadataLookup {

	private static Logger log = Logger.getLogger(PostgresCatalogLookup.class);
	protected GProMMetadataLookupPlugin plugin;
	private Connection con;
	private Statement stat;

	public AbstractMetadataLookup(Connection con) throws SQLException {
		this.con = con;
		createPlugin();
	}

	/**
	 * 
	 */
	private void createPlugin() {
		plugin = new GProMMetadataLookupPlugin ();
		plugin.databaseConnectionClose= new databaseConnectionClose_callback() {

			@Override
			public int apply() {
				return closeConnection();
			}
			
		};
		plugin.databaseConnectionOpen = new databaseConnectionOpen_callback() {

			@Override
			public int apply() {
				return openConnection();
			}
			
		};
	}

	private int openConnection () {
		try {
			stat = con.createStatement();
			return 0;
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
			return 1;
		}
	}
	
	
	private int closeConnection () {
		try {
			stat.close();
			return 0;
		}
		catch (SQLException e) {
			LoggerUtil.logException(e, log);
			return 1;
		}
	}
	
}
