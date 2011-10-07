package wd;

public class Timer {
	static class Times {
		long real;
		
		Times(long real) {
			this.real = real;
		}
	};

	
	private long timeBegin;


	Timer() {
		this.timeBegin = System.nanoTime();
	}

	Times getTimes() {
		long timeEnd = System.nanoTime();
		return new Times(timeEnd - timeBegin);
	}
}
