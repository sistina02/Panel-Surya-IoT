# Panel Surya IoT 🌞

**Panel Surya IoT** adalah proyek monitoring berbasis web untuk memantau kinerja panel surya fleksibel Axis dan statis. Sistem ini memungkinkan pemantauan parameter penting seperti waktu, sudut X, sudut Y, tegangan (voltage), daya (power), dan arus (current). Semua data disimpan di Firebase Realtime Database (RTDB) dan Firestore, serta divisualisasikan dalam bentuk grafik yang interaktif. Data tiap panel juga dapat diekspor dan disimpan dalam format CSV untuk keperluan analisis lebih lanjut.

## Fitur Utama

- **Monitoring Real-time** 🕒: Pantau data seperti sudut sumbu X dan Y, tegangan, daya, dan arus dari panel surya secara langsung melalui dashboard berbasis web.
- **Grafik Data** 📊: Visualisasi data performa panel surya dengan grafik interaktif, memudahkan pengguna dalam menganalisis efisiensi panel dari waktu ke waktu.
- **Penyimpanan History** 📚: Data disimpan di Firebase RTDB dan Firestore, memungkinkan akses ke riwayat performa panel surya untuk analisis jangka panjang.
- **Ekspor Data CSV** 📈: Pengguna dapat mengekspor data tiap panel surya ke dalam format CSV untuk dianalisis lebih lanjut di luar sistem.

## Teknologi yang Digunakan

- **Framework**: [Next.js](https://nextjs.org/) - untuk pengembangan frontend dan server-side rendering
- **Backend**: [Express.js](https://expressjs.com/) - untuk mengelola API dan komunikasi server-client
- **Frontend Styling**: [Tailwind CSS](https://tailwindcss.com/) - untuk desain dan tata letak antarmuka pengguna
- **Database**: [Firebase Realtime Database (RTDB)](https://firebase.google.com/products/realtime-database) dan [Firestore](https://firebase.google.com/products/firestore) - untuk penyimpanan dan pengambilan data secara real-time dan history
- **HTML**: Untuk membangun struktur halaman web dalam Next.js
- **Sistem Panel Surya**: C++/Arduino - digunakan untuk mengendalikan perangkat keras panel surya dan memproses data sensor

## Instalasi

1. Clone this repository:
   ```bash
   git clone https://github.com/sistina02/Panel-Surya-IoT.git
2. Go to the project directory:
   ```bash
   cd panel-surya-iot
3. Install dependencies:
   ```bash
   npm install
4. run the project locally:
   ```bash
   npm run dev

## Demo 🌐
Akses demo aplikasi di: https://panelsurya-edafc.web.app/

## Cara Kerja

- **Monitoring Data:** Aplikasi ini memonitor parameter seperti sudut X dan Y, tegangan, daya, serta arus dari panel surya. Data tersebut dikirimkan secara real-time ke Firebase dan divisualisasikan dalam bentuk grafik interaktif di dashboard web.
- **Penyimpanan Data:** Firebase RTDB menyimpan data real-time yang diterima dari perangkat, sementara Firestore menyimpan data historis untuk keperluan analisis performa jangka panjang.
- **Ekspor Data:** Pengguna dapat mengekspor data panel surya dalam format CSV untuk keperluan analisis lebih lanjut menggunakan software eksternal.

## Team
1. Mimi Azmita
2. Nabila Salvaningtyas
4. Rifki Tirta Nur Iman
5. Muhamad Ridwan Suryadi
6. Aditya Trias Putra
7. Eka Rachma Aprilidanti
8. Abdurrahman Al Faiz Al Atsary
9. Diva Anggraeni Fatmasari
10. Novandino Ramadhan
