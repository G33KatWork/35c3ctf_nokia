Test
====

Name    MSISDN  IMSI                Ki
Test1   1001    262230000000001     000102030405060708090a0b0c0d0e0f
Test2   1002    262230000000002     00112233445566778899aabbccddeeff
Target  9999    262230000000003     00102030405060708090a0b0c0d0e0f0

Commands to create HLR in omso-hlr:

    subscriber imsi 262230000000001 create
    subscriber imsi 262230000000001 update msisdn 1001
    subscriber imsi 262230000000001 update aud2g comp128v1 ki 000102030405060708090a0b0c0d0e0f

    subscriber imsi 262230000000002 create
    subscriber imsi 262230000000002 update msisdn 1002
    subscriber imsi 262230000000002 update aud2g comp128v1 ki 00112233445566778899aabbccddeeff

    subscriber imsi 262230000000003 create
    subscriber imsi 262230000000003 update msisdn 9999
    subscriber imsi 262230000000003 update aud2g comp128v1 ki 00102030405060708090a0b0c0d0e0f0


Prod
====

Name        MSISDN  IMSI                Ki
Attacker    1001    262230000000001     000102030405060708090a0b0c0d0e0f
Target      9999    262231111111111     46f7acaa234e0c4d696418f29b8ab278

Commands to create HLR in omso-hlr:

    subscriber imsi 262230000000001 create
    subscriber imsi 262230000000001 update msisdn 1001
    subscriber imsi 262230000000001 update aud2g comp128v1 ki 000102030405060708090a0b0c0d0e0f

    subscriber imsi 262231111111111 create
    subscriber imsi 262231111111111 update msisdn 9999
    subscriber imsi 262231111111111 update aud2g comp128v1 ki 46f7acaa234e0c4d696418f29b8ab278
