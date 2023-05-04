//
// Copyright: 2023 Suzuki Yoshinori(wave.suzuki.z@gmail.com)
// since: 2023/Apr.26
//
#include <SDL_events.h>
#include <array>
#include <cinttypes>
#include <draw.h>
#include <iostream>
#include <random>
#include <worker.h>

namespace
{
std::random_device                     rnd;
std::mt19937                           mtrand{rnd()};
std::uniform_real_distribution<double> rand{};

//
class MobWorker : public SimpleWorker::Worker
{
    std::array<double, 2> prev;
    std::array<double, 2> pos;
    std::array<double, 2> vel;
    std::array<double, 2> dispPos;

  public:
    MobWorker()
    {
        pos[0] = rand(mtrand);
        pos[1] = rand(mtrand);
        vel[0] = (rand(mtrand) * 2 - 0.5) * (1.0 / 60);
        vel[1] = (rand(mtrand) * 2 - 0.5) * (1.0 / 60);
        prev   = pos;
    }
    void run() override
    {
        prev = pos;
        for (int i = 0; i < 2; i++)
        {
            pos[i] += vel[i];
            if (pos[i] <= 0 || pos[i] >= 1)
            {
                vel[i] = -vel[i];
            }
        }
        dispPos[0] = pos[0] * (double)ScreenWidth;
        dispPos[1] = pos[1] * (double)ScreenHeight;
    }

    const std::array<double, 2>& getDispPos() const { return dispPos; }
};

// メインループ
class MainWorker : public SimpleWorker::ChainWorker
{
    class DrawWorker : public SimpleWorker::ChainWorker
    {
        std::vector<MobWorker>& moblist;

      public:
        DrawWorker(std::vector<MobWorker>& ml) : SimpleWorker::ChainWorker("Draw"), moblist(ml) {}

        SimpleWorker::count_t* dbgcnt;

        void run() override
        {
            auto* cnt = getCounter();
            Printf(100, 450, "%zu/%d", moblist.size(), cnt->load());

            SetColor(255, 255, 255);
            for (auto& mob : moblist)
            {
                auto& p = mob.getDispPos();
                DrawPoint(p[0], p[1]);
            }
        }
    };

    DrawWorker             drawWorker;
    std::vector<MobWorker> moblist;
    SimpleWorker::count_t  mobCounter;

  public:
    MainWorker(size_t n = 10000) : SimpleWorker::ChainWorker("Main"), drawWorker(moblist), moblist(n)
    {
        drawWorker.dbgcnt = &mobCounter;
    }
    void run() override
    {
        Print(100, 400, "Main Loop");

        mobCounter  = moblist.size();
        auto* owner = getOwner();
        for (auto& mob : moblist)
        {
            if (owner->push(mob, &mobCounter, "MOB") == false)
            {
                std::cout << "mob failed" << std::endl;
            }
        }

        drawWorker.attach(&mobCounter, *owner, getCounter());
    }
};

} // namespace

int
main(int argc, char** argv)
{
    CreateWindow("Multi Thread Draw");

    SimpleWorker::WorkerThread wt{20000, 8};
    SimpleWorker::count_t      mainCounter{};

    // フレームの最後に呼ばれる締めのワーカー
    class PostWorker : public SimpleWorker::Worker
    {
      public:
        void run() override { Print(10, 100, "Hello,World"); }
    };
    PostWorker postWorker{};
    MainWorker mainWorker{};

    bool onExecute = true;
    while (onExecute)
    {
        FrameUpdate();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                onExecute = false;
                break;
            }
        }

        mainWorker.attach(wt, &mainCounter);
        wt.push(&mainCounter, postWorker, nullptr, "POST");
        wt.execute();
        wt.wait();
        FrameSync();
    }

    DestroyWindow();
    return 0;
}
//
