#Job Shop Scheduling System
###CSE Data Structures Term Project – PROJ-17

---
##  Proje Amacı
- Üretim tesislerinde birden fazla işin, belirli makinelerde belirli sırayla işlenmesi problemine çözüm geliştirmek  
- Job–Operation–Machine yapısını veri yapıları ile doğru temsil etmek  
- Verilen operasyon sıralamasını kullanarak zaman çizelgesi (schedule) oluşturmak  
- Her operasyon için başlangıç ve bitiş zamanlarını hesaplamak  
- Üretilen çizelgenin geçerliliğini (feasibility) kontrol etmek  
- Makespan (toplam bitiş süresi) değerini hesaplamak ve minimize etmeye çalışmak  
- Scheduling heuristics ve local search yöntemleri ile daha iyi çözümler üretmek  
- Sonuçları görsel ve akademik olarak ifade edilebilir hale getirmek  

---

## Temel Kavramlar
- Makespan: Sistemdeki son operasyonun tamamlanma zamanı  
- Operation Precedence: Aynı iş içindeki operasyonların belirli sırada yapılma zorunluluğu  
- Machine Contention:  Aynı makineyi kullanmak isteyen operasyonlar arasında seçim yapma durumu  
- Critical Path:  Schedule’ın tamamlanma süresini belirleyen en uzun bağımlılık zinciri  
- Slack:  Bir operasyonun kritik yol dışında sahip olduğu zaman toleransı  
- Dispatching Rules: Hangi bekleyen operasyonun önce işleneceğini seçen stratejiler  
  - SPT (Shortest Processing Time)  
  - LPT (Longest Processing Time)  
  - FCFS (First Come First Served)  
  - Critical Path Priority  

---

## Proje Mimarisi

- Person 1 — Data Model & Input
  - Job, Operation ve Machine sınıflarının tasarımı  
  - JSON input okuma ve veri yapılarının oluşturulması  
  - Kısıtların veri yapılarıyla doğru temsil edilmesi  

---

- Person 2 — Schedule Engine
  - Operation sequence → zaman tabanlı schedule dönüştürme (schedule decoding)  
  - Başlangıç–bitiş zamanı hesaplama  
  - Makespan hesaplama  
  - Feasibility kontrolü  
    - Job precedence kontrolü  
    - Machine overlap kontrolü  

---

-  Person 3 — Dispatching Heuristics
  - Başlangıç schedule üretimi  
  - SPT, LPT, FCFS gibi dispatching heuristic’lerin uygulanması  
  - Heuristic sonuçlarının karşılaştırılması  
  - Uygun başlangıç çözümleri elde edilmesi  

---

- Person 4 — Critical Path & Local Search
  - Critical Path tespiti  
  - Slack hesaplama  
  - Schedule’ı analiz ederek makespan’i etkileyen kritik noktaların belirlenmesi  
  - Local Search ile iyileştirme (swap işlemleri, komşu çözüm üretimi)  
  - İyileştirilmiş schedule ve kritik yol analizi çıktıları üretimi  

---

- Person 5 — Frontend & Raporlama  - Gantt Chart çizimi  
  - Heuristic sonuçlarının görselleştirilmesi  
  - Deney sonuçlarının yorumlanması  
  - Final rapor ve sunum dokümanlarının hazırlanması  
  - Akademik ve anlaşılır çıktı sunumu

---
