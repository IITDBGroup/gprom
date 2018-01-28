/**
 * 
 */
package org.gprom.jdbc.utility;

import java.lang.reflect.Method;
import java.util.Collection;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.Logger;


/**
 *
 * Utilities for logging, mostly logging whole stack traces of exceptions.
 * 
 * @author Boris Glavic
 *
 */
public class LoggerUtil {

	public static void logException (Throwable e, Logger log) {
		log.error(e.toString() + "\n\n" + getCompleteTrace(e));
	}
	
	public static void logException (Exception e, Logger log, String message) {
		if (log.isEnabled(Level.ERROR))
		log.error(message + "\n\n" + getCompleteTrace(e));
	}
	
	public static void logDebugException (Throwable e, Logger log) {
		if (log.isDebugEnabled())
			if (log.isDebugEnabled()) {log.debug(getCompleteTrace(e));};
	}
	
	public static void logExceptionAndFail (Exception e, Logger log) {
		log.fatal(getCompleteTrace(e));
		//System.exit(1);
	}
	
	public static String getCompleteTrace (Throwable e) {
		StringBuilder trace;
		Throwable exception;
		
		trace = new StringBuilder ();
		trace.append(getStackString (e));
		exception = e;
		while (exception.getCause() != null) {
			exception = exception.getCause();
			trace.append("\ncaused by:\n");
			trace.append(getStackString (exception));
		}
		
		return trace.toString();
	}
	
	private static String getStackString (Throwable e) {
		StringBuilder stackString;
		StackTraceElement[] stack;
		
		stack = e.getStackTrace();
		stackString = new StringBuilder ();
		stackString.append("Exception occured: " + e.getClass().getName() + "\n");
		stackString.append("Message: " + e.getMessage() + "\n");
		for (int i = 0; i < stack.length; i++) {
			stackString.append(stack[i].toString() + "\n");
		}
		return stackString.toString();
	}
	
	public static String stringColToString (Collection<String> strings) {
		StringBuilder result;
		
		if (strings.isEmpty())
			return "";
		
		result = new StringBuilder();
		
		for(String elem: strings)
			result.append("'" + elem + "',");
		result.deleteCharAt(result.length() - 1);
		
		return result.toString();
	}
	
	public static String arrayToString (String[] array, String delim, boolean quotes) {
		StringBuilder result;
		String quot = quotes ? "'" : "";
		
		if (array.length == 0)
			return "";
		
		result = new StringBuilder();
		
		for (String elem: array) {
			result.append(quot + elem + quot + delim);
		}
		result.deleteCharAt(result.length() - 1);
		
		return result.toString();
	}
	
	public static String arrayToString (String[] array) {
		return arrayToString(array, ",", true);
	}
	
	public static void logArray (Logger log, Object[] array) {
		logArray(log, array, null);
	}
	
	public static void logArray (Logger log, int[] array, String message) {
		StringBuffer result = new StringBuffer();
		
		if (!log.isDebugEnabled())
			return;
		
		if (message != null)
			result.append(message + ":\n");
		
		for (int o : array) {
			result.append(o + ",");
		}
		
		result.deleteCharAt(result.length() - 1);
		
		if (log.isDebugEnabled()) {log.debug(result.toString());};
	}
		
	public static void logArray (Logger log, Object[] array, String message) {
		StringBuffer result;
		
		if (!log.isDebugEnabled())
			return;
		
		result = new StringBuffer();
		
		if (message != null)
			result.append(message + ":\n");
		
		for (Object o : array) {
			result.append(o.toString() + ",");
		}
		result.deleteCharAt(result.length() - 1);
		
		if (log.isDebugEnabled()) {log.debug(result.toString());};
	}
	
	public static void logObjectColWithMethod (Logger log, Collection<?> objs,
			Class<?> objClass, String methodName) throws  Exception {
		if (log.isDebugEnabled()) {log.debug(ObjectColToStringWithMethod(objs,objClass, methodName));};
	}
	
	public static String ObjectColToStringWithMethod (Collection<?> objs, 
			Class<?> objClass, String methodName) throws Exception {
		StringBuffer result = new StringBuffer();
		Method method;
		String callResult;
		
		method = objClass.getMethod(methodName);
		
		result.append('[');
		for(Object elem: objs) {
			if (result.length() != 1)
				result.append(',');
			callResult = (String) method.invoke(elem);
			result.append(callResult);
		}
		result.append(']');
		
		return result.toString();
	}
	
}
