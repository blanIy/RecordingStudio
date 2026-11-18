#include "DemoData.h"

void seedDemoData(Studio& st) {
    const int c1 = st.registerClient(Client(0, "Иван Петров", "+375291112233", "ivan@mail.by", "Северный ветер", 10));
    const int c2 = st.registerClient(Client(0, "Анна Соколова", "+375334445566", "anna@mail.by", "Anna S.", 0));
    st.registerClient(Client(0, "Дмитрий Орлов", "+375447778899", "orlov@mail.by", "Orlov Band", 25));

    const int e1 = st.registerEngineer(SoundEngineer(0, "Сергей Волков", "+375291000001", "volkov@studio.by", "Запись вокала", 25.0, 8));
    st.registerEngineer(SoundEngineer(0, "Мария Ким", "+375291000002", "kim@studio.by", "Сведение и мастеринг", 35.0, 12));
    const int p1 = st.registerProducer(Producer(0, "Олег Лис", "+375291000003", "lis@studio.by", "рок", 20.0));

    const int mic = st.registerEquipment(Equipment(0, "Neumann U87", EquipmentType::MIC, 15.0, false));
    const int cons = st.registerEquipment(Equipment(0, "SSL Nucleus", EquipmentType::CONSOLE, 0.0, true));
    const int mon = st.registerEquipment(Equipment(0, "Genelec 8040", EquipmentType::MONITOR, 0.0, true));
    const int gtr = st.registerEquipment(Equipment(0, "Fender Telecaster", EquipmentType::INSTRUMENT, 10.0, false));
    const int mic2 = st.registerEquipment(Equipment(0, "Shure SM7B", EquipmentType::MIC, 0.0, false));

    const int r1 = st.registerRoom(StudioRoom(0, "Зал A (запись)", 40.0));
    const int r2 = st.registerRoom(StudioRoom(0, "Зал B (сведение)", 30.0));
    st.installEquipment(r1, mic);
    st.installEquipment(r1, cons);
    st.installEquipment(r1, gtr);
    st.installEquipment(r2, mon);
    st.installEquipment(r2, mic2);

    const int pr1 = st.createProject(Project(0, c1, "Первый альбом", "рок"));
    st.createProject(Project(0, c2, "Сингл «Лето»", "поп"));
    st.advanceProject(pr1, ProjectStatus::RECORDING);

    SessionData d;
    d.clientId = c1; d.engineerId = e1; d.roomId = r1; d.projectId = pr1; d.producerId = p1;
    d.start = DateTime(2026, 12, 1, 10, 0);
    d.durationHours = 4;
    d.rentedEquipmentIds = {mic, gtr};
    const int s1 = st.planSession(SessionType::RECORDING, d, 6);
    st.confirmSession(s1);
    st.issueInvoice(s1);
}
