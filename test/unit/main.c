int zmtp_msg_tests(void);
int zmtp_dealer_tests(void);

int
main(int argc, char* argv[])
{
    ((void)argc);
    ((void)argv);
    int err = 0;

    err |= zmtp_msg_tests();
    err |= zmtp_dealer_tests();

    return err;
}
