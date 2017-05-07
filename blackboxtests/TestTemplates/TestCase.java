package PACKAGE;

import java.sql.SQLException;

import org.junit.Test;
import org.junit.runners.MethodSorters;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.gprom.jdbc.test.testgenerator.AbstractGProMTester;
import org.gprom.jdbc.test.testgenerator.ConnectionOptions;
import org.gprom.jdbc.test.testgenerator.OptionsManager;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class $NAME extends AbstractGProMTester {

	public $NAME (String name) {
		super (name);
		try {
			path = "$PATH";
			ConnectionOptions.getInstance().setPath("$PATH");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	protected void setUp () throws Exception {
		path = "$BASEDIR";
		ConnectionOptions.getInstance().setPath("$PATH");
		super.setUp();
		setFile();
	}
	
	protected void tearDown () throws Exception {
		super.tearDown();
	}
	
    public void setFile () throws SQLException, Exception {
		setGenerator("$FILE");
		OptionsManager.getInstance().setOptions (SETTING);
    }
	
	TESTS
	
}