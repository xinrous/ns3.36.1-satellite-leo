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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes; // NodeContainer ��һ�������ڵ������
  nodes.Create (2); // NodeContainer �ܹ�ͨ������ Create ���������ڵ㣬����ڵ���������

  PointToPointHelper pointToPoint; // ׼��ʹ�� PointToPointHelper �������ŵ�����
  pointToPoint.SetDeviceAttribute ("DataRate",
                                   StringValue ("5Mbps")); // �����ŵ��� DataRate ����������
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms")); // �����ŵ��� Delay �ӳ�����

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes); // ���忴Install������ע��

  // Ϊ�ڵ㰲װTCP/IPЭ�����������InternetStackHelper
  InternetStackHelper stack;
  // stack ������� Install ����֮���Ϊ���ǵĽڵ㰲װ��Э��ջ
  stack.Install (nodes);
  // �ⰲװ��Э��ջ�Ľڵ㻹���ܹ�����ֱ�ӵ�ͨ�ţ�����ҪΪ�����豸����IP��ַ
  Ipv4AddressHelper address;
  // ���SetBase������ֹ������һ����ַ�����Ƿ�����һ�ε�ַ
  address.SetBase ("10.1.1.0", "255.255.255.0");
  // Ipv4AddressHelperΪ���ǵ�devices�ϵĽӿڽ��е�ַ�ķ��䣬Ȼ�󷵻ؽӿ�
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  // ����һ������Ӧ�ò������������9�Ŷ˿ڣ�����1s��ʼ���м�����10s����
  // ------------------------------------------------------------------
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  // ------------------------------------------------------------------

  // ����һ������Ӧ�ò�ͻ��ˣ�Ŀ�ص�ַΪ1�Žڵ�ӿڵĵ�ַ��Ŀ�Ķ˿�Ϊ9
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  // ���Է������������Ե����ã�������������,���鷢�ͼ���������С����2s��ʼ���ͣ�10sֹͣ
  // ------------------------------------------------------------------
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  // ------------------------------------------------------------------

  // ����֮ǰ���趨������в���
  Simulator::Run ();
  // Destroyִ������Ĳ���
  Simulator::Destroy ();
  // C++ģ��ű���������һ��main��������Ҫ����0���߲���ϵͳ����ִ�гɹ�
  return 0;
}
