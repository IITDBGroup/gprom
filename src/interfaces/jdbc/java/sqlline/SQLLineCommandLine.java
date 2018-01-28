package sqlline;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.gprom.jdbc.utility.PropertyConfigurator;

import sqlline.SqlLine;
import sqlline.SqlLine.Status;

/**
 * @author lord_pretzel
 *
 */
public class SQLLineCommandLine extends SqlLine {

	public static final String[] GPROM_SQLLINE_ARGS = {"-d", "org.gprom.jdbc.driver.GProMDriver"};
	public static final String INFO_TEXT = 
			"********************************************************************************\n"
			+ "* sqlline version 1.3.0 for GProM JDBC\n\n"
			+ "\t- connect to a database with !connect, \n"
			+ "\t\te.g., to connect to an sqlite database <test.db> use !connect jdbc:gprom:sqlite:test.db\n"
			+ "\t- use !quit to quit\n"
			+ "********************************************************************************\n";
	
	private SqlLine sqlline;
	private String[] origArgs;
	
	public static void main(String[] args) throws IOException {
		PropertyConfigurator.configureHonoringProperties("log4j2-test.xml", "log4j2.xml");
		SQLLineCommandLine cl = new SQLLineCommandLine(args);
		Status status = cl.start();
		System.exit(status.ordinal());
	}
	
	@Override
	public String getPrompt() {
		String cmd = "gprom-" + super.getPrompt();
		return cmd;
	}
	
	@Override
	public String getApplicationTitle() {
		return INFO_TEXT;
	}
	
	public SQLLineCommandLine (String args[]) {
		super();
		this.origArgs = args;
	}

	private Status start () throws IOException {
		String[] args;
	    args = createArgs(origArgs);
	    Status status = super.begin(args, null, true);
	    
	    return status;
	}
	
	/**
	 * @param args
	 * @return
	 */
	private String[] createArgs(String[] args) {
		List<String> argList = new ArrayList<String> ();
		argList.addAll(Arrays.asList(args));
		argList.addAll(Arrays.asList(GPROM_SQLLINE_ARGS));
		return argList.toArray(new String[] {});
	}
	
	
}
