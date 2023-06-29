//
// Created by gels on 2023/6/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "enet/enet.h"
#include <string.h>
#include <memory.h>

int main() {
    fprintf(stderr, "=====start testclient====\n");
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    ENetHost * client;

    client = enet_host_create (NULL /* create a client host */,
                               1 /* only allow 1 outgoing connection */,
                               2 /* allow up 2 channels to be used, 0 and 1 */,
                               0 /* assume any amount of incoming bandwidth */,
                               0 /* assume any amount of outgoing bandwidth */);

    if (client == NULL)
    {
        fprintf (stderr,
                 "An error occurred while trying to create an ENet client host.\n");
        exit (EXIT_FAILURE);
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    /* Connect to some.server.net:1234. */
    enet_address_set_host (& address, "127.0.0.1");
    address.port = 1234;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (client, & address, 2, 0);

    if (peer == NULL)
    {
        fprintf (stderr,"No available peers for initiating an ENet connection.\n");
        exit (EXIT_FAILURE);
    }

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client, & event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        puts ("Connection to some.server.net:1234 succeeded.");

        int end = 0;
        int i = 0;
        while(1) {

            char str2[24];
            memset(str2, 0, sizeof(str2));
            sprintf(str2, "foo%d", i);
            fprintf(stdout, "client do while loop i=%d len_str2=%d\n", i, strlen(str2));
            i++;
            /* Create a reliable packet of size 7 containing "packet\0" */
            char str[] = "packet_";
            ENetPacket * packet = enet_packet_create (str,
                                                      strlen (str) + 1,
                                                      ENET_PACKET_FLAG_RELIABLE);
            /* Extend the packet so and append the string "foo", so it now */
            /* contains "packetfoo\0"                                      */
            enet_packet_resize (packet, strlen (str) + strlen(str2) + 1);
            strcpy (& packet -> data [strlen (str)], str2);

            /* Send the packet to the peer over channel id 0. */
            /* One could also broadcast the packet by         */
            /* enet_host_broadcast (host, 0, packet);         */
            enet_peer_send (peer, 0, packet);
            /* One could just use enet_host_service() instead. */
            enet_host_flush (client);

            while (enet_host_service(client, &event, 0) > 0) {
                switch (event.type) {
                    case ENET_EVENT_TYPE_RECEIVE:
                        printf("client receive packet of length= %u data= %s was received from %s on channel %u.\n",
                               event.packet->dataLength,
                               event.packet->data,
                               event.peer->data,
                               event.channelID);

                        /* Clean up the packet now that we're done using it. */
                        enet_packet_destroy(event.packet);
                        break;

                    case ENET_EVENT_TYPE_DISCONNECT:
                        printf("client %s disconnected.\n", event.peer->data);

                        /* Reset the peer's client information. */
                        event.peer->data = NULL;
                        end = 1;
                        break;
                }
            }

            if(end) {
                break;
            }
            usleep(100000);
        }
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer);

        puts ("Connection to some.server.net:1234 failed.");
    }



    enet_host_destroy(client);

    atexit (enet_deinitialize);
    return 0;
}

