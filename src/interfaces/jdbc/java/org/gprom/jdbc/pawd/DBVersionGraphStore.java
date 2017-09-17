package org.gprom.jdbc.pawd;

import java.util.Properties;

public class DBVersionGraphStore implements VersionGraphStore {
    private static final String PROP_STORE_JDBC_DRIVER ="JDBC_DRIVER";
    private static final String PROP_STORE_DB_URL ="DB_URL";
    private static final String PROP_STORE_USER ="USER";
    private static final String PROP_STORE_PASS ="PASS";
    static final String WRITE_OBJECT_SQL = "INSERT INTO java_objects(name, object_value) VALUES (?, ?)";

    @Override
    public void initialize(Properties options) throws IllegalArgumentException {
        String JDBC_DRIVER;
        if (options.getProperty(PROP_STORE_JDBC_DRIVER) == null)
            throw new IllegalArgumentException ("need to provide option " + PROP_STORE_JDBC_DRIVER);
        else
            JDBC_DRIVER = options.getProperty(PROP_STORE_JDBC_DRIVER);
        String DB_URL;
        if (options.getProperty(PROP_STORE_DB_URL) == null)
            throw new IllegalArgumentException ("need to provide option " + PROP_STORE_DB_URL);
        else
            DB_URL = options.getProperty(PROP_STORE_DB_URL);
        String USER;
        if (options.getProperty(PROP_STORE_USER) == null)
            throw new IllegalArgumentException ("need to provide option " + PROP_STORE_USER);
        else
            USER = options.getProperty(PROP_STORE_USER);
        String PASS;
        if (options.getProperty(PROP_STORE_PASS) == null)
            throw new IllegalArgumentException ("need to provide option " + PROP_STORE_PASS);
        else
            PASS = options.getProperty(PROP_STORE_PASS);
    }

    @Override
    public VersionGraph load(String versionGraphId) {
        // TODO implement
        return null;
    }

    @Override
    public void save(VersionGraph g) {
        // TODO implement


    }
}
