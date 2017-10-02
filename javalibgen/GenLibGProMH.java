import java.io.File;
import java.io.FileOutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import com.ochafik.lang.jnaerator.JNAeratorConfig.Runtime;
import com.ochafik.lang.jnaerator.ClassOutputter;
import com.ochafik.lang.jnaerator.JNAerator;
import com.ochafik.lang.jnaerator.JNAeratorCommandLineArgs;
import com.ochafik.lang.jnaerator.JNAeratorConfig;
import com.ochafik.lang.jnaerator.Result;
import com.ochafik.lang.jnaerator.SourceFiles;
import com.ochafik.lang.jnaerator.JNAerator.Feedback;
import com.ochafik.lang.jnaerator.JNAeratorConfig.OutputMode;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;

public class GenLibGProMH {

	public static void main(String[] args) {
		if(args.length != 1){
			System.err.println("Wrong number of args: usage: GenLibGProMH sourceRootDir");
			System.exit(1);
		}
		String srcRoot = args[0];	
		GenLibGProMH libGProMHeaderGenerator = new GenLibGProMH(srcRoot);
		libGProMHeaderGenerator.generateLibGProMEntries();
	}
	
	private String srcRoot;
	
	public static String libraryNamePrefix = "GProM";
	private static String libraryDestinationHeaderFile = "include/libgprom/libgprom.h";
	private static String javaSrcDestinationDir = "src/interfaces/jdbc/java/";
	private static String javaLibraryDestinationFile = javaSrcDestinationDir + "org/gprom/jdbc/jna/GProM_JNA.java";
	private static String queryOperatorDir = "include/model/query_operator/query_operator.h";
	private static String queryBlockDir = "include/model/query_block/query_block.h";
	private static String nodeDir = "include/model/node/nodetype.h";
	private static String listDir = "include/model/list/list.h";
	private static String expressionDir = "include/model/expression/expression.h";
	private static String uthashDir = "include/uthash.h";
	private static String hashmapDir = "include/model/set/hashmap.h";
	private static ArrayList<LibraryItem> libraryItems = new ArrayList<LibraryItem>(Arrays.asList( 
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "AttributeDef"),
			new StructInternalUnionLibraryItem(GenLibGProMH.listDir, "ListCell"),
			new StructLibraryItem(GenLibGProMH.listDir, "List"),
			new StructLibraryItem(GenLibGProMH.nodeDir, "Node"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "AttributeReference"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "Schema"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "QueryOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "TableAccessOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "SelectionOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "ProjectionOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "JoinOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "AggregationOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "SetOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "DuplicateRemoval"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "WindowOperator"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "Constant"),
			new StructLibraryItem(GenLibGProMH.queryBlockDir, "ProvenanceTransactionInfo"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "ProvenanceComputation"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "UpdateOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "ConstRelOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "NestingOperator"),
			new StructLibraryItem(GenLibGProMH.queryOperatorDir, "OrderOperator"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "FunctionCall"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "Operator"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "SQLParameter"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "RowNumExpr"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "CaseExpr"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "CaseWhen"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "IsNullExpr"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "WindowBound"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "WindowFrame"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "WindowDef"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "WindowFunction"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "CastExpr"),
			new StructLibraryItem(GenLibGProMH.expressionDir, "OrderExpr"),
			new StructLibraryItem(GenLibGProMH.nodeDir, "KeyValue"),
			new StructLibraryItem(GenLibGProMH.uthashDir, "UT_hash_bucket"),
			new StructLibraryItem(GenLibGProMH.uthashDir, "UT_hash_table"),
			new StructLibraryItem(GenLibGProMH.uthashDir, "UT_hash_handle"),
			new StructLibraryItem(GenLibGProMH.hashmapDir, "HashElem"),
			new StructLibraryItem(GenLibGProMH.hashmapDir, "HashMap"),
			new EnumWithToStringLibraryItem(GenLibGProMH.nodeDir, "NodeTag"),
			new EnumWithToStringLibraryItem(GenLibGProMH.expressionDir, "DataType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.queryBlockDir, "JoinType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.queryBlockDir, "SetOpType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.nodeDir, "ProvenanceType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.nodeDir, "ProvenanceInputType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.queryBlockDir, "IsolationLevel"),
			new EnumWithToStringLibraryItem(GenLibGProMH.queryBlockDir, "NestingExprType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.expressionDir, "WindowBoundType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.expressionDir, "WinFrameType"),
			new EnumWithToStringLibraryItem(GenLibGProMH.expressionDir, "SortOrder"),
			new EnumWithToStringLibraryItem(GenLibGProMH.expressionDir, "SortNullOrder")
	));
	
	public GenLibGProMH(String srcRoot){
		this.srcRoot = srcRoot;
	}
	
	public void generateLibGProMEntries(){
		System.err.println("---------------------------------- generateLibGProMEntries ---------------------------------");
		try{
			Iterator<String> libItemReplacementsIter;
			String itemReplacementItemStr, itemReplacementItemStrR;
			Iterator<LibraryItem> libItemIter = libraryItems.iterator();
			ArrayList<String> libraryItemReplacements =  new ArrayList<String>();
			while(libItemIter.hasNext()){
				libraryItemReplacements.add(libItemIter.next().getName());
			}
			libItemIter = libraryItems.iterator();
			LibraryItem currentItem;
			String srcFileContents;
			String dstFileContents = new String(Files.readAllBytes(Paths.get(GenLibGProMH.libraryDestinationHeaderFile)));
			Pattern libItemPattern;
			Matcher libItemMatcher;
			Pattern libDstItemPattern;
			Matcher libDstItemMatcher;
			String destItemString;
			String srcToJNAerate = "";
			while(libItemIter.hasNext()){
				currentItem = libItemIter.next();
				srcFileContents = new String(Files.readAllBytes(Paths.get(currentItem.getFileName())));
				libItemPattern = Pattern.compile(currentItem.getSrcMatchRegex());
				libItemMatcher = libItemPattern.matcher(srcFileContents);
			    if(libItemMatcher.find()) {
			    	destItemString = currentItem.getDstReplacementString(libItemMatcher);
			    	libItemReplacementsIter = libraryItemReplacements.iterator();
			    	while(libItemReplacementsIter.hasNext()){
			    		itemReplacementItemStr = libItemReplacementsIter.next();
			    		itemReplacementItemStrR = "\\b"+itemReplacementItemStr+"\\b";
			    		destItemString = destItemString.replaceAll(itemReplacementItemStrR, GenLibGProMH.libraryNamePrefix + itemReplacementItemStr);
			    	}
			    	srcToJNAerate += destItemString + "\n"; 
			    	//System.err.println(destItemString);
			    	//System.err.println("---------------------------------");
			    	libDstItemPattern = Pattern.compile(currentItem.getDstMatchRegex());
			    	libDstItemMatcher = libDstItemPattern.matcher(dstFileContents);
			    	if(libDstItemMatcher.find()) {
			    		dstFileContents = dstFileContents.replaceFirst(currentItem.getDstMatchRegex(), destItemString);
			    	}
			    	else{
			    		System.err.println("---------------------------------");
				    	System.err.println("----Missing Dest Library Item----");
				    	System.err.println("----------"+currentItem.getName()+"---------");
				    	System.err.println("----------Adding As New----------");
				    	dstFileContents = dstFileContents.replace("#endif /* INCLUDE_LIBGPROM_LIBGPROM_H_ */", destItemString + "\n\n#endif /* INCLUDE_LIBGPROM_LIBGPROM_H_ */");
			    	}	
			    }
			    else{
			    	System.err.println("---------------------------------");
			    	System.err.println("-------Missing Library Item------");
			    	System.err.println("----------"+currentItem.getName()+"---------");
			    	System.err.println("---------------------------------");
			    }
			}
			//System.err.println(dstFileContents);
			writeLibGproMHeaderFile(dstFileContents);
			jnaerateJavaFiles(srcToJNAerate.replaceAll("(\\b)char\\s*\\*\\s*", "$1String "));
			modifyGProM_JNA();
		}
		catch(Exception e){
			System.err.println(e.toString());
		}
	}	
	
	private void jnaerateJavaFiles(String cSrcStr){
		JNAeratorConfig config = new JNAeratorConfig();
        config.outputJar = new File("jnaerator_temp.jar");
        config.outputDir = new File("");
        config.sourcesOutputDir = new File(this.srcRoot + GenLibGProMH.javaSrcDestinationDir);
        config.useJNADirectCalls = false;
        //config.forceNames = false;
        config.forceOverwrite = true;
        config.putTopStructsInSeparateFiles = true;
        config.reification = false;
        config.convertBodies = false;
        config.genRawBindings = false;
        config.beautifyNames = false;
        //config.scalaStructSetters = scalaSettersCb.isSelected();
        config.stringifyConstCStringReturnValues = true;
        config.charPtrAsString = true;
        config.runtime = Runtime.JNA;
        config.outputMode = OutputMode.Directory;
        config.noComments = false;
        config.defaultLibrary = "org.gprom.jdbc.jna";
        config.libraryForElementsInNullFile = "org.gprom.jdbc.jna";
        config.preprocessorConfig.includeStrings.add(cSrcStr);
        if (config.runtime == Runtime.BridJ) {
            config.genCPlusPlus = true;
        } else {
            config.genCPlusPlus = config.genCPlusPlus || cSrcStr.contains("//@" + JNAeratorCommandLineArgs.OptionDef.CPlusPlusGen.clSwitch);
        }
        config.cacheDir = new File("cache");
		
		Feedback feedback = new Feedback() {
             @Override
             public void setStatus(final String string) {}
             @Override
             public void setFinished(final File toOpen) {}
             @Override
             public void setFinished(Throwable e) {}
             @Override
             public void wrappersGenerated(final Result result) {}
             @Override
             public void sourcesParsed(SourceFiles sourceFiles) {}
         };
         
		new JNAerator(config) {
            public PrintWriter getSourceWriter(final ClassOutputter outputter, final String path) throws IOException {
               try{
            	   System.err.println("Generated Library Item: Path: "+config.sourcesOutputDir+path);
            	   File file = new File(config.sourcesOutputDir, path);
                   File parent = file.getParentFile();
                   if (!parent.exists()) {
                       parent.mkdirs();
                   }
                   feedback.setStatus("Generating " + file.getName()); 
            	   return newFileWriter(file);
                   
               }
            	catch(Exception e){
            		System.err.println("---------------------------------------");
        	    	System.err.println("----------Exception JNAerating---------");
        	    	System.err.println("----------"+e.toString()+"---------");
        	    	System.err.println("---------------------------------------");
            	}
            	return null;
            }
            PrintWriter newFileWriter(File file) throws IOException {
                if (file.exists()) {
                    if (config.forceOverwrite ) {
                        System.out.println("Overwriting file '" + file + "'");
                    } else {
                        throw new IOException("File '" + file + "' already exists (use forceOverwrite = true; to overwrite).");
                    }
                }
                file.getAbsoluteFile().getParentFile().mkdirs();
                return new PrintWriter(file) {
                	@Override
                    public void write(String s, int off, int len)  {
                		try{
                        	String replacedOutput = s.replaceAll("\\/\\*\\*\\s+\\*\\s+This file was autog[a-zA-Z0-9<>.:\\/\\\\,_\\s*=\"]+\\*\\/", "");
                        	// ---------a couple of replacement hacks for specific cases :  I should make this better------
                        	replacedOutput = replacedOutput.replaceAll("import org\\.gprom\\.jdbc\\.jna\\.OrgGpromJdbcJnaLibrary\\.String;", "");
                        	replacedOutput = Pattern.compile("(public data_union\\([\\sa-zA-Z].*?)\\}", Pattern.DOTALL).matcher(replacedOutput).replaceAll("$1\twrite();\n\t\t}\n");
                        	replacedOutput = Pattern.compile("(public static class ByValue extends data_union.*?)\\};",Pattern.DOTALL).matcher(replacedOutput).replaceAll( "$1\tpublic ByValue(Pointer ptr_value) {\n\t\t\t\tsuper(ptr_value);\n\t\t\t}\n\t\t\tpublic ByValue(int int_value) {\n\t\t\t\tsuper(int_value);\n\t\t\t}\n\t\t};" );
                        	replacedOutput = Pattern.compile("(public static class ByReference extends GProMNode.*?)\\};",Pattern.DOTALL).matcher(replacedOutput).replaceAll( "$1\tpublic ByReference() { super(); }\n\t\tpublic ByReference(com.sun.jna.Pointer p) { super(p); }\n\t};" );
                        	// --------------------------------------------------------------------------------------------
                        	Pattern classNamePattern = Pattern.compile("public class ([a-zA-Z_]+) extends Structure");
	                        Matcher classNameMatcher = classNamePattern.matcher(replacedOutput);
	                        if(classNameMatcher.find()){
	                    		String className = classNameMatcher.group(1);
	                    		replacedOutput = replacedOutput.replaceFirst(classNameMatcher.group(), classNameMatcher.group().replaceFirst("extends Structure", "extends GProMStructure"));
	                    		
	                    		Pattern classConstructorPattern = Pattern.compile("public "+className+"\\s*\\(([a-zA-Z0-9\\s,._]+)\\)\\s*\\{([\\sa-zA-Z0-9,._;=()]+)");
	                            Matcher classConstructorMatcher = classConstructorPattern.matcher(replacedOutput);
	                            if(classConstructorMatcher.find()){
	                            	String constructorArgs = classConstructorMatcher.group(1);
	                            	String constructorArgNames =  "";
	                            	Pattern classConstructorArgNamesPattern = Pattern.compile(" ([a-zA-Z0-9_]+\\s*[,)])");
	                            	Matcher classConstructorArgNamesMatcher = classConstructorArgNamesPattern.matcher(constructorArgs+")");
	                            	while(classConstructorArgNamesMatcher.find()){
	                            		constructorArgNames += classConstructorArgNamesMatcher.group(1);
	                            	}
	                            	replacedOutput = replacedOutput.replace(classConstructorMatcher.group(), classConstructorMatcher.group().replace(classConstructorMatcher.group(2), classConstructorMatcher.group(2) + "\twrite();\n\t" ));
	                        		
	                            	Pattern classDefaultConstructorPattern = Pattern.compile("public "+className+"\\s*\\(\\s*\\)\\s*\\{([\\sa-zA-Z0-9,._;=()]+)");
		                            Matcher classDefaultConstructorMatcher = classDefaultConstructorPattern.matcher(replacedOutput);
		                            if(classDefaultConstructorMatcher.find()){
		                            	replacedOutput = replacedOutput.replace(classDefaultConstructorMatcher.group(), classDefaultConstructorMatcher.group()+"}\n\tpublic "+className+"(com.sun.jna.Pointer address){\n\t\tsuper(address);\n\t");
		                            }
	                            	
	                            	Pattern byValuePattern = Pattern.compile("public static class ByValue extends ([a-zA-Z]+) implements Structure\\.ByValue \\{");
	    	            			Matcher byValueMatcher = byValuePattern.matcher(replacedOutput);
	    	                    	if(byValueMatcher.find()){
	    	                    		replacedOutput = replacedOutput.replace(byValueMatcher.group(), byValueMatcher.group() + "\n\t\tpublic ByValue(" + constructorArgs + "){\n\t\t\tsuper("+constructorArgNames + ";\n\t\t}");
	    	                    	}
	                            }
	                        }
	                        /*System.err.println("-----------------------------------");
	         		    	System.err.println("-------JNAerated Replaced Library Item------");
	         		    	System.err.println(replacedOutput);
	         		    	System.err.println("---------------------------------");
	                        */
	                        super.write(replacedOutput, off, replacedOutput.length());
	                	}
	                	catch(Exception e){
	                		StringWriter sw = new StringWriter();
	                		PrintWriter pw = new PrintWriter(sw);
	                		for(StackTraceElement st : e.getStackTrace()){
	                			pw.println(st.toString());
	                		}
	                		System.err.println("---------------------------------------");
	            	    	System.err.println("----------Exception JNAerating---------");
	            	    	System.err.println("----------"+e.toString()+"---------");
	            	    	System.err.println("----------"+sw.toString()+"---------");
	            	    	System.err.println("---------------------------------------");
	            	    	
	                	}
                    }
                };
            }
		}.jnaerate(feedback);
	}
	
	private void modifyGProM_JNA(){
		try{
			String srcFileContents = new String(Files.readAllBytes(Paths.get(GenLibGProMH.javaSrcDestinationDir + "org/gprom/jdbc/jna/OrgGpromJdbcJnaLibrary.java")));
			Pattern generatedContentPattern = Pattern.compile("public static interface .*?};", Pattern.DOTALL);
	        Matcher generatedContentMatcher = generatedContentPattern.matcher(srcFileContents);
	        String generatedContent = "";
	        while(generatedContentMatcher.find()){
	        	generatedContent += "\t" + generatedContentMatcher.group() + "\n\n";
	        }
	        	
			String dstFileContents = new String(Files.readAllBytes(Paths.get(GenLibGProMH.javaLibraryDestinationFile)));
			Pattern replacePattern = Pattern.compile("public static interface .*};", Pattern.DOTALL);
	        Matcher replaceMatcher = replacePattern.matcher(dstFileContents);
	        if(replaceMatcher.find()){
	        	dstFileContents = dstFileContents.replace(replaceMatcher.group(), generatedContent.trim());
	        	File libgpromJavaFile = new File(GenLibGProMH.javaLibraryDestinationFile);
				FileOutputStream libgpromJavaFOS = new FileOutputStream(libgpromJavaFile, false); // true to append
				byte[] contentBytes = dstFileContents.getBytes(); 
				libgpromJavaFOS.write(contentBytes);
				libgpromJavaFOS.close();
	        }
	        else{
	        	System.err.println("---------------------------------------");
		    	System.err.println("----Problem Writing Java Lib File----");
		    	System.err.println("----------"+"No Match"+"---------");
		    	System.err.println("---------------------------------------");
	        }
			File srcFileToDel = new File(GenLibGProMH.javaSrcDestinationDir + "org/gprom/jdbc/jna/OrgGpromJdbcJnaLibrary.java");
			srcFileToDel.delete();
		}
        catch(Exception e){
			System.err.println("---------------------------------------");
	    	System.err.println("----Exception Writing Java Lib File----");
	    	System.err.println("----------"+e.toString()+"---------");
	    	System.err.println("---------------------------------------");
		}
	}
	
	public static void writeLibGproMHeaderFile(String contents){
		try{
			File libgpromhFile = new File(GenLibGProMH.libraryDestinationHeaderFile);
			FileOutputStream libgpromhFOS = new FileOutputStream(libgpromhFile, false); // true to append
			byte[] contentBytes = contents.getBytes(); 
			libgpromhFOS.write(contentBytes);
			libgpromhFOS.close();
		}
		catch(Exception e){
			System.err.println("---------------------------------------");
	    	System.err.println("-------Exception Writing Lib File------");
	    	System.err.println("----------"+e.toString()+"---------");
	    	System.err.println("---------------------------------------");
		}
	}
}



abstract class LibraryItem {
	protected String name;
	private String fileName; 
	public LibraryItem(String fileName, String name){
		this.fileName = fileName;
		this.name = name;
	}
	public String getName(){
		return this.name;
	}
	public String getFileName(){
		return this.fileName;
	}
	public abstract String getSrcMatchRegex();
	public abstract String getDstMatchRegex();
	public abstract String getDstReplacementString(Matcher libItemMatcher);
}

class StructLibraryItem extends LibraryItem {
	public StructLibraryItem(String fileName, String name){
		super(fileName, name);
	}
	public String getSrcMatchRegex(){
		return "typedef\\s+struct\\s+("+this.name+")(\\s*\\{\\s+[\\sa-zA-Z0-9*,;\\/().'_<>+#-]+\\}\\s*)("+this.name+")\\s*;"; 
	}
	public String getDstMatchRegex(){
		return "typedef\\s+struct\\s+("+GenLibGProMH.libraryNamePrefix+this.name+")(\\s*\\{\\s+[\\sa-zA-Z0-9*,;\\/().'_<>+#-]+\\}\\s*)("+GenLibGProMH.libraryNamePrefix+this.name+")\\s*;"; 
	}
	public String getDstReplacementString(Matcher libItemMatcher){
		String matchedString = libItemMatcher.group();
		matchedString = matchedString.replaceAll("boolean\\s+", "int ");
		matchedString = matchedString.replaceAll("ptrdiff_t\\s+", "int ");
		return matchedString.replaceAll(libItemMatcher.group(1), GenLibGProMH.libraryNamePrefix+libItemMatcher.group(1));
	}
}


class StructInternalUnionLibraryItem extends LibraryItem {
	public StructInternalUnionLibraryItem(String fileName, String name){
		super(fileName, name);
	}
	public String getSrcMatchRegex(){
		return "typedef\\s+struct\\s+("+this.name+")(\\s*\\{\\s+[\\sa-zA-Z0-9*,\\{\\};_\\/().'<>+#-]+\\}\\s*)("+this.name+")\\s*;"; 
	}
	public String getDstMatchRegex(){
		return "typedef\\s+struct\\s+("+GenLibGProMH.libraryNamePrefix+this.name+")(\\s*\\{\\s+[\\sa-zA-Z0-9*,\\{\\};_\\/().'<>+#-]+\\}\\s*)("+GenLibGProMH.libraryNamePrefix+this.name+")\\s*;"; 
	}
	public String getDstReplacementString(Matcher libItemMatcher){
		String matchedString = libItemMatcher.group();
		matchedString = matchedString.replaceAll("boolean\\s+", "int ");
		matchedString = matchedString.replaceAll("ptrdiff_t\\s+", "int ");
		return matchedString.replaceAll(libItemMatcher.group(1), GenLibGProMH.libraryNamePrefix+libItemMatcher.group(1));
	}
}


class EnumLibraryItem extends LibraryItem {
	public EnumLibraryItem(String fileName, String name){
		super(fileName, name);
	}
	public String getSrcMatchRegex(){
		return "(typedef\\s+enum\\s*("+this.name+")?\\s*\\{(\\s*)[\\sa-zA-Z0-9,*;\\/.'_]+\\s*)\\}\\s*("+this.name+");";
	}
	public String getDstMatchRegex(){
		return "typedef\\s+enum\\s*("+GenLibGProMH.libraryNamePrefix+this.name+")?\\s*\\{(\\s*[\\sa-zA-Z0-9,*;\\/.'_]+\\s*)\\}\\s*("+GenLibGProMH.libraryNamePrefix+this.name+");";
	}
	public String getDstReplacementString(Matcher libItemMatcher){
		String bodyGroup;
		if(libItemMatcher.groupCount() == 6)
			bodyGroup = libItemMatcher.group(2);
		else
			bodyGroup = libItemMatcher.group(1);
		return libItemMatcher.group(1) + bodyGroup.replaceAll("(\\s*)([a-zA-Z0-9_,}\\/*]+)\n", "$1"+GenLibGProMH.libraryNamePrefix+"_$2\n") + "} " + this.name + ";";
	}
}

class EnumWithToStringLibraryItem extends LibraryItem {
	public EnumWithToStringLibraryItem(String fileName, String name){
		super(fileName, name);
	}
	public String getSrcMatchRegex(){
		return "NEW_ENUM_WITH_TO_STRING\\s*\\(("+this.name+")\\s*,(\\s*[\\sa-zA-Z0-9,*;\\/.'_]+\\s*)\\)\\s*;";
	}
	public String getDstMatchRegex(){
		return "typedef\\s+enum\\s*("+GenLibGProMH.libraryNamePrefix+this.name+")?\\s*\\{(\\s*[\\sa-zA-Z0-9,*;\\/.'_]+\\s*)\\}\\s*("+GenLibGProMH.libraryNamePrefix+this.name+");";
	}
	public String getDstReplacementString(Matcher libItemMatcher){
		String bodyGroup = libItemMatcher.group(2);
		return "typedef enum " + GenLibGProMH.libraryNamePrefix+this.name + "{\n" +  bodyGroup.replaceAll("(\\s*)([a-zA-Z0-9_,]+)\n", "$1"+GenLibGProMH.libraryNamePrefix+"_$2\n") + "} " + this.name + ";";
	}
}



