#include "OTTripleSetup.h"

void OTTripleSetup::setup()
{
    timeval baseOTstart, baseOTend;
    gettimeofday(&baseOTstart, NULL);

    G.ReSeed();
    for (int i = 0; i < nbase; i++)
    {
        base_receiver_inputs[i] = G.get_uchar() & 1;
    }
    //baseReceiverInput.randomize(G);

    for (int i = 0; i < nparties - 1; i++)
    {
        baseOTs[i]->set_receiver_inputs(base_receiver_inputs);
        baseOTs[i]->exec_base(false);
        baseSenderInputs[i] = baseOTs[i]->sender_inputs;
        baseReceiverOutputs[i] = baseOTs[i]->receiver_outputs;
    }
    gettimeofday(&baseOTend, NULL);
#ifdef VERBOSE
    double basetime = timeval_diff(&baseOTstart, &baseOTend);
    cout << "\t\tBaseTime: " << basetime/1000000 << endl << flush;
#endif

    for (size_t i = 0; i < baseOTs.size(); i++)
    {
        delete baseOTs[i];
    }

    // Receiver send something to force synchronization
    // (since Sender finishes baseOTs before Receiver)
}

void OTTripleSetup::close_connections()
{
    for (size_t i = 0; i < players.size(); i++)
    {
        delete players[i];
    }
}
