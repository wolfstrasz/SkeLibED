template<typename IN, typename OUT>
class Scoreboard {
public:
    // Data for full task
    std::vector<OUT> *output;
    std::vector<IN> *input;
    size_t inputSize;
    // State of the full task
    bool isFinished;
    bool isInitialised;
    // Data for next thread job
    size_t curIndex;
    size_t jobSize;
    // Guard lock for thread communication
    std::mutex scoreboardLock;

    // constructor
    Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads) {
        this->output = out;
        this->input = in;
        isFinished = false;
        isInitialised = false;
        inputSize = in->size();
        curIndex = 0;
        jobSize = 0;
    }

    void GetNewTask(double workTime, size_t &elementsCount, size_t &elementIndex){
            // Convert time to milliseconds
            workTime = workTime / 1000000.0f;

            // Mean elements should be always rounded up
            this->jobSize = (elementsCount / workTime) + 0.5f;

            // Get the data for the new Job
            if (this->curIndex + this->jobSize < this->inputSize) {
                elementsCount = this->jobSize;
                elementIndex = this->curIndex;
                this->curIndex += this->jobSize;
            }
            else {
                elementsCount = this->inputSize - this->curIndex;
                elementIndex = this->curIndex;
                this->curIndex += elementsCount;
                this->isFinished = true;
            }
    }
    ~Scoreboard() {}
};