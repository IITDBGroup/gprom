package org.gprom.jdbc.test.testgenerator;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.InvalidPropertiesFormatException;
import java.util.Locale;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org. gprom.jdbc.test.testgenerator.dataset.DataAndQueryGenerator;
import org.stringtemplate.v4.NumberRenderer;
import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.STGroup;
import org.stringtemplate.v4.STGroupFile;
import org.stringtemplate.v4.StringRenderer;

public class TestGenerator {

	Logger log = LogManager.getLogger(TestGenerator.class);
	
	public static int settingNum;
	
	public static  String PACKAGE_NAME;
	private static String packageDir;
	public static String resourceDir;
	public static String testcaseDir;
	
	private HashMap<String,GProMSuite> suites;
	private File testDir;
	private String packageName;
	private GProMSuite allTests;
	private STGroup g;
	
	public class MyIntRenderer extends NumberRenderer {
	    @Override
	    public String toString(Object o, String formatString, Locale locale) {
	        if (formatString == null || !formatString.startsWith("pad"))
	            return super.toString(o, formatString, locale);
	       // pad integer from left with 0's
	        int padLen = Integer.parseInt(formatString.substring(4, formatString.length() - 1));
	        String thing = o.toString();
	        while(thing.length() < padLen)
	        	thing = "0" + thing;
	        return thing;
	    }
	}
	
	public TestGenerator (File testDir, String packageName) throws IOException {

		this.testDir = testDir;
		ConnectionOptions.getInstance().setPath(testDir.getPath());
		OptionsManager.getInstance().reloadOptions();
		File dir;
		
		this.packageName = packageName; 
		
		packageDir = System.getProperty("generator.sourcedir") + "/" + packageName.replaceAll("\\.", "/");
		dir = new File(packageDir);
		if (!dir.isDirectory())
			dir.mkdirs();
		
		g = new STGroupFile(resourceDir + "/TestTemplates/testcase.stg");
	}
	
	public static void main (String[] args) throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		TestGenerator gen;
		File dir;
		PACKAGE_NAME = System.getProperty("generator.package");
		packageDir = System.getProperty("generator.sourcedir");
		packageDir += "/" + PACKAGE_NAME.replace('.', '/');
		resourceDir = System.getProperty("generator.resourcedir");
		testcaseDir = System.getProperty("generator.testcasedir");
		dir = new File (testcaseDir);
		gen = new TestGenerator (dir, PACKAGE_NAME);
		gen.generateTests();
		gen.generateOptionsSuites();
	}
	
	public void generateTests () throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		for (int i = 0; i < OptionsManager.getInstance().getNumSettings(); i++) {
			settingNum = i + 1;
			
			suites = new HashMap<String,GProMSuite> ();
			allTests = new GProMSuite ("AllTests_" + settingNum);
			suites.put("allTests_" + settingNum, allTests);
			//allTests.addChild(new GProMSuite("SetOptions_" + (i + 1)));
			
			generateTestRun();
		}
	}
	
	public void generateTestRun () throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		File[] files;
		DataAndQueryGenerator generator;
		String name;
		
		files = testDir.listFiles();
		
		for (int i = 0; i < files.length; i++) {
			name = files[i].getName();
			
			if (name.compareTo("template.xml") != 0 && name.compareTo("settings.xml") != 0  && name.endsWith(".xml")) {
				name = name.substring(0, name.length() - 4);
				
				generateSuitesFromFileName (name);
				
				System.out.println("create Generator for " + name);
				generator = new DataAndQueryGenerator (files[i].getAbsolutePath());
				generateTest (generator, name);	
			}
		}
		
		finalizeSuites ();
	}
	
	public void generateOptionsSuites () throws InvalidPropertiesFormatException, FileNotFoundException, IOException {
		GProMSuite optionSuite;
		
		optionSuite = new GProMSuite ("AllTestsOptions");
		
		for (int i = 0; i < OptionsManager.getInstance().getNumSettings(); i++) {
			generateSetOption (i + 1);
			optionSuite.addChildWithDupes(new GProMSuite("AllTests_" + (i + 1)));
		}
		
		ST suiteclassfile = g.getInstanceOf("suiteclassfile");
		suiteclassfile.add("package", packageName);
		suiteclassfile.add("name", optionSuite.getName());
		for(GProMSuite child: optionSuite.getChildren())
			suiteclassfile.add("child", child.getClassName());
		
		writeFile(optionSuite.getName(), suiteclassfile.render());
	}
	
	private void generateSetOption (int optionNum) throws IOException {	
		ST optionsclass = g.getInstanceOf("optionsclassfile");
		optionsclass.add("num", optionNum);
		optionsclass.add("package", packageName);
		
		writeFile("SetOptions_" + optionNum, optionsclass.render());
	}
	
	private void finalizeSuites () throws IOException {
		java.util.Iterator<String> iter;
		GProMSuite suite;
		
		allTests.addChild(new GProMSuite("ReportPrinter"));
		iter = suites.keySet().iterator();
		
		while (iter.hasNext()) {
			suite = suites.get(iter.next());
			
			if (suite.getChildren().size() > 0) {
				
				ST suiteclassfile = g.getInstanceOf("suiteclassfile");
				suiteclassfile.add("package", packageName);
				suiteclassfile.add("name", suite.getClassName());
				for(GProMSuite child: suite.getChildren())
					suiteclassfile.add("child", child.getClassName());
								
				writeFile(suite.getClassName(), suiteclassfile.render());
			}
		}
	}
	
	
	
	private void generateTest (DataAndQueryGenerator generator, String name) throws IOException {
		GProMSuite suite;
		String runName;
		
		runName = generateName (name);
		suite = suites.get(runName);
		
		STGroup g = new STGroupFile(resourceDir + "/TestTemplates/testcase.stg");
		g.registerRenderer(Integer.class, new MyIntRenderer());
		ST testcl = g.getInstanceOf("testclass");
		testcl.add("name", suite.getClassName());
		testcl.add("file", suite.getFileName());
		testcl.add("setting", settingNum);
		testcl.add("path", this.getTestDir().toString() + "/");
		testcl.add("basedir", resourceDir);
		
		for (int i = 1; i <= generator.getNumTest(); i++) {
			if (!generator.isInExcludes(settingNum, i))
				testcl.add("casenum", i);
		}
		
		ST testclassfile = g.getInstanceOf("testclassfile");
		testclassfile.add("package", packageName);
		testclassfile.add("class", testcl.render());
		log.error(testclassfile.render());
		
		writeFile(suite.getClassName(), testclassfile.render());
	}
	
	private String padInt (int in, int len) {
		String res = Integer.toString(in);
		while(res.length() < len)
			res = "0" + res;
		return res;
	}
	
	private String generateName (String name) {
		String[] parts;
		String result;
		
		parts = name.split("\\.");
		
		for (int i = 0; i < parts.length; i++) {
			parts[i] = parts[i] + "_" + settingNum;
		}
		
		result = parts[0];
		for (int i = 1; i < parts.length; i++) {
			result = result + "." + parts[i];
		}
		
		result += "_Test";
		
		return result;
	}
	
	private void generateSuitesFromFileName (String fileName) {
		String[] parts;
		String curName;
		GProMSuite curSuite;
		GProMSuite oldSuite;
		
		System.out.println("create Suites for " + fileName + " " + settingNum);
		
		parts = fileName.split("\\.");
		curName = "";
		oldSuite = allTests;
		
		for (int i = 0; i < parts.length; i++) {
			parts[i] = parts[i] + "_" + settingNum;
		}
		
		for (int i = 0; i < parts.length; i++) {
			if (curName.compareTo("") == 0) {
				curName = parts[i];
			}
			else {
				curName = curName + "." + parts[i];
			}
			if (i == parts.length -1) {
				curName += "_Test";
			}
			if (suites.containsKey(curName)) {
				curSuite = suites.get(curName);
			}
			else {
				curSuite = new GProMSuite (curName);
			}
			
			suites.put(curName, curSuite);
			oldSuite.addChild(curSuite);
			oldSuite = curSuite;
		}
	}
	
	private void writeFile (String name, String content) throws IOException {
		File outFile;
		FileWriter writer;
		
		outFile = new File (packageDir + "/" + name + ".java");
		System.out.println(outFile.getAbsolutePath());
		outFile.createNewFile();
		
		writer = new FileWriter (outFile);
		writer.write(content);
		writer.close();
	}
	
//	private String readString (String fileName) throws IOException {
//		File file;
//		FileReader reader;
//		BufferedReader bufRead;
//		StringBuffer result;
//		
//		result = new StringBuffer ();
//		file = new File (fileName);
//		reader = new FileReader (file);
//		bufRead = new BufferedReader (reader);
//		
//		while (bufRead.ready()) {
//			result.append(bufRead.readLine() + "\n");
//		}
//		
//		bufRead.close();
//		reader.close();
//		
//		return result.toString();
//	}

	
	public File getTestDir () {
		return testDir;
	}

	
	public void setTestDir (File testDir) {
		this.testDir = testDir;
	}
}
