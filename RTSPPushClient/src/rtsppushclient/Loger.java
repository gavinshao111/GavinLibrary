package rtsppushclient;

public class Loger {
	public static final int LOGER_DEBUG = 0;
	public static final int LOGER_INFO  = 1;
	public static final int LOGER_ERROR = 2;
	public static final int LOGER_FATAL = 3;
	
	private static int logerLevel = LOGER_DEBUG;


	public static void DEBUG(String str)
	{
		
		if (logerLevel <= LOGER_DEBUG)
		{
			System.out.println(str);
		}
	}
	
	public static void INFO(String str)
	{
		if (logerLevel <= LOGER_INFO)
		{
			System.out.println(str);
		}
	}
	
	public static void ERROR(String str)
	{
		if (logerLevel <= LOGER_ERROR)
		{
			System.out.println(str);
		}
	}
	
	public static void FATAL(String str)
	{
		if (logerLevel <= LOGER_FATAL)
		{
			System.out.println(str);
		}
	}
	
	public static void EXCEPTION(Exception exc)
	{
            exc.printStackTrace();
	}

}
