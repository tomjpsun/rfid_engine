rfidengine v0.4.2 說明:

本說明使用適用於 __開發環境__的，如果僅需要 __使用__
請參考 scripts 子目錄的 install.sh

本軟體是用於 AidiaLink 的客戶端, 負責與 RFID Reader
連線, 對它下命令, 以執行 tags 的動作, 包括:

	1. 讀取 EPC, TID and User data
	2. 寫入 EPC，User data


How to build?
	開發系統 Debian 10 (Buster)


Prerequite Packages:
	gcc or clang, make


Build:
	需要 sudo.

	安裝 rfid_engine lib:
		$> make
		$> sudo make install
	建立 Cpp test:
		$> make test

	Compiler options:
	CXXFLAGS+= -std=c++17 -I /usr/local/include/rfid_engine
	$(CXX) $(CXXFLAGS) -Wl,-rpath,/usr/local/lib/ -o [your exec] [your cpp] -pthread -lrfid_engine

	Then check with command:
	$> ldd [your exec]
	You should see librfid_engine.so.xx, like the following:

	tom@omega:~/work/rfid_engine$ ldd test

	linux-vdso.so.1 (0x00007ffc4b5f0000)
	libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f8929659000)
	librfid_engine.so.0.4 => /usr/local/lib/librfid_engine.so.0.4 (0x00007f8929322000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f892919f000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f8929185000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f8929164000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f8928fa3000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f8929849000)


#