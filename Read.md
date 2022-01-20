rfidengine 說明:

版本請參考 version.mk.

本說明使用適用於 __設置__ 開發環境 ，如果僅需要 __使用__
請參考 scripts 子目錄的 install.sh

本軟體是用於 AidiaLink 的客戶端, 負責與 RFID Reader
連線, 對它下命令, 以執行 tags 的動作, 包括:

	1. 讀取 EPC, TID and User data
	2. 寫入 EPC，User data


How to build?
	本程式可以在下列開放環境佈署：
	X86 PC/ Debian 10 (Buster)
	Raspberry Pi 4/ Manjaro

Prerequite Packages:
	gcc or clang, make

自帶第三方套件:
	asio, nlohmann, aixlog

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

Distribution:

	make install

	會將設定檔放在 /etc/rfidengine/rfid_config.json
	並且將 librfidengine-[version].so copy 到 /usr/local/lib
	與 header files copy 到 /usr/local/include/rfidengine

#
