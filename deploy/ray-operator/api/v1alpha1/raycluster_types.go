package v1alpha1

import (
	v1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

// EDIT THIS FILE!  THIS IS SCAFFOLDING FOR YOU TO OWN!
// NOTE: json tags are required.  Any new fields you add must have json tags for the fields to be serialized.

// RayClusterSpec defines the desired state of RayCluster
type RayClusterSpec struct {
	// INSERT ADDITIONAL SPEC FIELDS - desired state of cluster
	// Important: Run "make" to regenerate code after modifying this file
	ClusterName     string          `json:"clusterName"`
	Images          RayClusterImage `json:"images"`
	ImagePullPolicy string          `json:"imagePullPolicy,omitempty"`
	Extensions      []Extension     `json:"extensions,omitempty"`
}

// RayClusterStatus defines the observed state of RayCluster
type RayClusterStatus struct {
	// INSERT ADDITIONAL STATUS FIELD - define observed state of cluster
	// Important: Run "make" to regenerate code after modifying this file
}

// +genclient
// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// +kubebuilder:object:root=true

// RayCluster is the Schema for the RayClusters API
type RayCluster struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   RayClusterSpec   `json:"spec,omitempty"`
	Status RayClusterStatus `json:"status,omitempty"`
}

// +k8s:deepcopy-gen:interfaces=k8s.io/apimachinery/pkg/runtime.Object

// RayClusterList contains a list of RayCluster
type RayClusterList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata,omitempty"`
	Items           []RayCluster `json:"items"`
}

func init() {
	SchemeBuilder.Register(&RayCluster{}, &RayClusterList{})
}

type RayClusterImage struct {
	DefaultImage string `json:"defaultImage,omitempty"`
}

type Extension struct {
	// the replicas of this pod group
	Replicas int32 `json:"replicas"`

	// worker/head
	Type string `json:"type,omitempty"`

	// pod image
	Image string `json:"image,omitempty"`

	// logical groupName for worker in same group, it's used for heterogeneous feature to distinguish different groups
	// groupName is the unique identifier for the pods with the same configuration in one Ray Cluster
	GroupName string `json:"groupName,omitempty"`

	// command to start ray
	Command string `json:"command,omitempty"`

	// labels for pod, raycluster.component and rayclusters.ray.io/component-name are default labels, do not overwrite them.
	// Labels are intended to be used to specify identifying attributes of objects that are meaningful and relevant to users,
	// but do not directly imply semantics to the core system. Labels can be used to organize and to select subsets of objects.
	Labels map[string]string `json:"labels,omitempty"`

	// NodeSelector specifies a map of key-value pairs. For the pod to be eligible
	// to run on a node, the node must have each of the indicated key-value pairs as
	// labels.Optional
	NodeSelector map[string]string `json:"nodeSelector,omitempty"`

	// the affinity for pod.Optional
	// it allows you to constrain which nodes your pod is eligible to be scheduled on, based on labels on the node.
	Affinity *v1.Affinity `json:"affinity,omitempty"`

	// the resource requirements for this group pod.
	Resources v1.ResourceRequirements `json:"resources,omitempty"`

	// Tolerations specifies the pod's tolerations.Optional
	// Tolerations are applied to pods, and allow (but do not require) the pods to schedule onto nodes with matching taints.
	Tolerations []v1.Toleration `json:"tolerations,omitempty"`

	// List of environment variables to set in the container.Optional
	// define environment variables for a container. This can be helpful in defining variables before setting up the container.
	ContainerEnv []v1.EnvVar `json:"containerEnv,omitempty"`

	// Head service suffix. So head can be accessed by domain name: {namespace}.svc , follows Kubernetes standard
	HeadServiceSuffix string `json:"headServiceSuffix,omitempty"`

	// Annotations for pod.Optional
	// usage refers to https://kubernetes.io/docs/concepts/overview/working-with-objects/annotations/
	Annotations map[string]string `json:"annotations,omitempty"`

	// Volume for the pod group.Optional
	// At its core, a volume is just a directory, possibly with some data in it, which is accessible to the Containers in a Pod.
	Volumes []v1.Volume `json:"volumes,omitempty"`

	// VolumeMount for the pod group.Optional
	// mount the volume to a path via name
	VolumeMounts []v1.VolumeMount `json:"volumeMounts,omitempty"`
}
