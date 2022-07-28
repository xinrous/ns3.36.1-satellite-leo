/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

/**
 * @brief third�ű���wifi����������һ������㣨Access-Point AP���� nWifi ���ƶ��ڵ���ɡ�
 * ��������APҲ��һ��˫ģ�ڵ㡣����װ��WIFI��PPP���������豸��
 * 
 * ���������WifiNetDevice��֮�⣬WIFI�����豸��������·�㣨WifiMac��������㣨WifiPhy���������
 * WifiNetDeviceֻ����һ���������²�Э������ã�û��ʲôʵ���ԵĹ��ܣ���Ҫ��wifiЭ�鹦�ܶ�������wifimac��
 * wifiphy���������Լ����������֮��ʵ�֡����ֽṹ���Խ�һ�����ӵĹ���ģ�黯�������ں����Ŀ�����ά������һ����
 * ����ʵ���ӷ�����������Ĳ�λ��֣�����netdevice+mac+phy��3�������豸�ṹ��ns-3�����߽ڵ��лᾭ��ʹ�õ���
 * */

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3; // 3���ƶ��ڵ�
  bool tracing = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc, argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
                << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  // ��1�� ����Channel��WifiPhy
  // �������඼����һЩ��Ҫ�Ĳ�����Ҫ�������ã��������ǵ�WifiChanenel��Ҫ���ô�
  // ���ӳٺͽ��չ��ʡ�WifiPhy��Ҫ����������ģ�ͣ����û�����������Ҫ������ʹ��
  // Ĭ�ϵ����ü��ɡ�
  // ---------------------------------------------------------------
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());
  // ---------------------------------------------------------------

  // ����wifimac ���ڽڵ�֮�а�װNetDevice
  // WifiMacHelper ����������·���WfifMac�࣬������Ҫ��������������Ҫ�Ĳ���
  // ����1�� wifimac���� ����2�����񼯱�ʶ��ssid
  // ǰ�߾���������ڵ��������AP�����ƶ��ڵ㣬���߾����˽ڵ������ķ��񼯡�
  // AP���ƶ��ڵ�ķ���һ�²��ܹ�����ͨ�š�
  // ---------------------------------------------------------------
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");

  // ���wifi�ŵ��������豸�и����齨������֮�󣬾Ϳ���ʹ��������WifiHelper��wifi�豸
  // ��װ��ָ���Ľڵ�֮��
  WifiHelper wifi;

  // �洢���е��ƶ��豸
  NetDeviceContainer staDevices;
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  // ��phy,mac���а�װ
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  // �洢���е�AP�豸
  NetDeviceContainer apDevices;
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  // ��phy,mac���а�װ
  apDevices = wifi.Install (phy, mac, wifiApNode);
  // ---------------------------------------------------------------

  // �����ƶ�ģ��
  MobilityHelper mobility;

  // �ƶ�ģ��������������һ���ز����ٵĲ��֡�
  // ������wifi channel��wifiphyʱ��һ��
  // ������Channel�еĴ����ӳٺͽ��չ��ʵĴ�
  // С���ɴ����ӳ�ģ�ͣ����ģ�ͺ��ƶ�ģ�͹�ͬ�����ġ�
  // �����ӳٺ����ģ����ǰ���Ѿ����ã����ƶ�ģ��һ����
  // wifi�����豸��װ���������á�������Ϊ��ͬwifi���
  // ������Ҫ���ò�ͬ���ƶ�ģ�͡�

  // ��һ���ƶ��ڵ���ƶ�ģ�͵�����
  // �ƶ��ڵ���ƶ�ģ�����÷�Ϊ���µ��������֣���ʼλ�÷ֲ��ͺ����ƶ��켣ģ��
  // ǰ�߶�����һ���ƶ��ڵ�ĳ�ʼ���꣬ʹ�õĳ�ʼλ�÷ֲ��������ǵ�GridPositionAllocator,����ֲ��������趨�õ��к��в������ڵ�Ⱦ���ķ���һ����ά�ѿ�������ϵ�С�
  // ���߶����˽ڵ���ƶ�·����ʹ�õ��ƶ��켣ģ����RandomWalk2MobilityModel��
  // ���ģ���еĽڵ���һ���ƶ���С�ĳ����������ڰ���������ٶȺͷ�������ƶ���
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY",
                                 DoubleValue (0.0), "DeltaX", DoubleValue (5.0), "DeltaY",
                                 DoubleValue (10.0), "GridWidth", UintegerValue (3), "LayoutType",
                                 StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds",
                             RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  // AP�ڵ��ǹ̶��Ľڵ㣬�˴�Ϊ��ʹ�õ��ƶ�ģ���ǹ̶�λ���ƶ�ģ�ͣ����ģ�͵�AP�ڵ��ά����Ϊ(0,0)
  // �������ԭ�㡣
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  // ������е���tcp-ipЭ��صİ�װ
  // ------------------------------------------------------------------------------

  InternetStackHelper stack;
  stack.Install (csmaNodes); // Ϊ���߽ڵ���а�װ
  stack.Install (wifiApNode); // Ϊ����AP�ڵ���а�װ
  stack.Install (wifiStaNodes); // Ϊ�����ƶ��ڵ���а�װ

  // ʹ��Ipv4AddressHelper���е�ַ������
  Ipv4AddressHelper address;

  // ����p2p�豸��ͬһ������
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  // �����ǵ�csma�豸����Ϊһ������
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  // ��AP�豸�����ǵ��ƶ��ڵ�����Ϊһ������
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  // ------------------------------------------------------------------------------

  // ����Ӧ�ó���İ�װ
  // ------------------------------------------------------------------------------
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  // ------------------------------------------------------------------------------

  // ����·�ɵ�����
  // ��ns3�Ķ������֮�����ͨ�ŵ�ʱ�����Ǿ���Ҫ�ڽű�������·��Э�顣���磬ǰ���ڴ���CSMA�����
  // ʱ����ᵽһ��p2pNodes.Get(1)������ڵ����PointToPoint��CSMA���������豸���ֱ�����PointToPoint����
  // �Լ����ǵ�CSMA���硣�ڵ�ַ����֮�У���������������ڲ�ͬ�������������Ҫ��������������p2pNodes.Get(1)
  // ����·�ɵĹ��ܣ��������ܹ���ȷ��ת����PPP�������͸�CSMA�����ķ��飬��֮��Ȼ
  //
  // ns3����õ�·��Э��֮һ����ȫ��·�ɣ�ȫ��·��ͨ������ʽ���·������·���㷨��������������ÿ�����ڵ�����·��
  // ����Ϊÿ���ڵ�����·�ɱ�����IPV4Э�飬ȫ��·������ֻҪ�������������е��ü��ɡ�
  // ------------------------------------------------------------------------------
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // ------------------------------------------------------------------------------
  Simulator::Stop (Seconds (10.0));

  // ����׷��
  // �����һ����ҪĿ�ľ��Ƿ����������ܣ���ȡʵ�������Ƿ����������ܵ���Ҫǰ�ᣬns3Ϊ�û��ṩ�˷ḻ������׷�ٺ�
  // �ռ��Ĺ��ܣ���ns3ģ��ű�֮�У��ռ��������ݾ�����pcap����ascii�ı��ĸ�ʽ������ָ�����ļ�֮�У��ⲿ�ִ���
  // �������ڴ����ĩβ
  // ------------------------------------------------------------------------------
  if (tracing)
    {
      phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
      // EnablePcapAll���������ռ�����ŵ������нڵ����·������շ���¼����¼�ļ��ĸ�ʽΪpcap
      // ����������ļ�������������Ϊ��ǰ׺��-�ڵ���-�����豸��š�
      pointToPoint.EnablePcapAll ("third");
      // ��ӡAP�ڵ���wifi�����豸���������շ���¼
      phy.EnablePcap ("third", apDevices.Get (0));
      // ��ӡһ�����߽ڵ���CSMA�����豸�����շ���¼
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
      // ע�⣺����tcpdump��wireshark���Ҳ���Դ�pcap�ļ�
    }
  // ------------------------------------------------------------------------------

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
